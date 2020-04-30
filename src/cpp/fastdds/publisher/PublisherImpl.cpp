// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * PublisherImpl.cpp
 *
 */

#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/topic/TopicDescriptionImpl.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/dds/log/Log.hpp>

#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <functional>

namespace eprosima {
namespace fastdds {
namespace dds {

using fastrtps::xmlparser::XMLProfileManager;
using fastrtps::xmlparser::XMLP_ret;
using fastrtps::rtps::InstanceHandle_t;
using fastrtps::Duration_t;
using fastrtps::PublisherAttributes;

static void set_qos_from_attributes(
        DataWriterQos& qos,
        const PublisherAttributes& attr)
{
    qos.writer_resource_limits().matched_subscriber_allocation = attr.matched_subscriber_allocation;
    qos.properties() = attr.properties;
    qos.throughput_controller() = attr.throughputController;
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
    qos.endpoint().history_memory_policy = attr.historyMemoryPolicy;
    qos.endpoint().user_defined_id = attr.getUserDefinedID();
    qos.endpoint().entity_id = attr.getEntityID();
    qos.reliable_writer_qos().times = attr.times;
    qos.reliable_writer_qos().disable_positive_acks = attr.qos.m_disablePositiveACKs;
    qos.durability() = attr.qos.m_durability;
    qos.durability_service() = attr.qos.m_durabilityService;
    qos.deadline() = attr.qos.m_deadline;
    qos.latency_budget() = attr.qos.m_latencyBudget;
    qos.liveliness() = attr.qos.m_liveliness;
    qos.reliability() = attr.qos.m_reliability;
    qos.lifespan() = attr.qos.m_lifespan;
    qos.user_data().setValue(attr.qos.m_userData);
    qos.ownership() = attr.qos.m_ownership;
    qos.ownership_strength() = attr.qos.m_ownershipStrength;
    qos.destination_order() = attr.qos.m_destinationOrder;
    qos.representation() = attr.qos.representation;
    qos.publish_mode() = attr.qos.m_publishMode;
    qos.history() = attr.topic.historyQos;
    qos.resource_limits() = attr.topic.resourceLimitsQos;
}

PublisherImpl::PublisherImpl(
        DomainParticipantImpl* p,
        const PublisherQos& qos,
        PublisherListener* listen)
    : participant_(p)
    , qos_(&qos == &PUBLISHER_QOS_DEFAULT ? participant_->get_default_publisher_qos() : qos)
    , listener_(listen)
    , publisher_listener_(this)
    , user_publisher_(nullptr)
    , rtps_participant_(p->rtps_participant())
    , default_datawriter_qos_(DATAWRITER_QOS_DEFAULT)
{
    PublisherAttributes pub_attr;
    XMLProfileManager::getDefaultPublisherAttributes(pub_attr);
    set_qos_from_attributes(default_datawriter_qos_, pub_attr);
}

void PublisherImpl::disable()
{
    set_listener(nullptr);
    user_publisher_->set_listener(nullptr);
    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        for (auto it = writers_.begin(); it != writers_.end(); ++it)
        {
            for (DataWriterImpl* dw : it->second)
            {
                dw->disable();
            }
        }
    }
}

PublisherImpl::~PublisherImpl()
{
    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        for (auto it = writers_.begin(); it != writers_.end(); ++it)
        {
            for (DataWriterImpl* dw : it->second)
            {
                delete dw;
            }
        }
        writers_.clear();
    }

    delete user_publisher_;
}

const PublisherQos& PublisherImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t PublisherImpl::set_qos(
        const PublisherQos& qos)
{
    if (&qos == &PUBLISHER_QOS_DEFAULT)
    {
        const PublisherQos& default_qos = participant_->get_default_publisher_qos();
        if (!can_qos_be_updated(qos_, default_qos))
        {
            return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
        }
        set_qos(qos_, default_qos, false);

        std::lock_guard<std::mutex> lock(mtx_writers_);
        for (auto topic_writers : writers_)
        {
            for (auto writer : topic_writers.second)
            {
                writer->publisher_qos_updated();
            }
        }

        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t ret_val = check_qos(qos);
    if (!ret_val)
    {
        return ret_val;
    }
    if (!can_qos_be_updated(qos_, qos))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }
    set_qos(qos_, qos, false);

    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto topic_writers : writers_)
    {
        for (auto writer : topic_writers.second)
        {
            writer->publisher_qos_updated();
        }
    }

    return ReturnCode_t::RETCODE_OK;
}

const PublisherListener* PublisherImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t PublisherImpl::set_listener(
        PublisherListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

void PublisherImpl::PublisherWriterListener::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_publication_matched(writer, info);
    }
}

void PublisherImpl::PublisherWriterListener::on_liveliness_lost(
        DataWriter* writer,
        const LivelinessLostStatus& status)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_liveliness_lost(writer, status);
    }
}

void PublisherImpl::PublisherWriterListener::on_offered_deadline_missed(
        DataWriter* writer,
        const fastrtps::OfferedDeadlineMissedStatus& status)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_offered_deadline_missed(writer, status);
    }
}

DataWriter* PublisherImpl::create_datawriter(
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listener,
        const StatusMask& mask)
{
    logInfo(PUBLISHER, "CREATING WRITER IN TOPIC: " << topic->get_name());
    //Look for the correct type registration
    TypeSupport type_support = participant_->find_type(topic->get_type_name());

    /// Preconditions
    // Check the type was registered.
    if (type_support.empty())
    {
        logError(PUBLISHER, "Type: " << topic->get_type_name() << " Not Registered");
        return nullptr;
    }

    if (!DataWriterImpl::check_qos(qos))
    {
        return nullptr;
    }

    topic->get_impl()->reference();

    DataWriterImpl* impl = new DataWriterImpl(
        this,
        type_support,
        topic,
        qos,
        listener);

    if (impl->writer_ == nullptr)
    {
        logError(PUBLISHER, "Problem creating associated Writer");
        delete impl;
        topic->get_impl()->dereference();
        return nullptr;
    }

    DataWriter* writer = new DataWriter(impl, mask);
    impl->user_datawriter_ = writer;

    //REGISTER THE WRITER
    WriterQos wqos = qos.get_writerqos(qos_, topic->get_qos());
    fastrtps::TopicAttributes topic_att = DataWriterImpl::get_topic_attributes(qos, *topic, type_support);
    rtps_participant_->registerWriter(impl->writer_, topic_att, wqos);

    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        writers_[topic->get_name()].push_back(impl);
    }

    return writer;
}

ReturnCode_t PublisherImpl::delete_datawriter(
        DataWriter* writer)
{
    if (user_publisher_ != writer->get_publisher())
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }
    std::lock_guard<std::mutex> lock(mtx_writers_);
    auto vit = writers_.find(writer->get_topic()->get_name());
    if (vit != writers_.end())
    {
        auto dw_it = std::find(vit->second.begin(), vit->second.end(), writer->impl_);
        if (dw_it != vit->second.end())
        {
            (*dw_it)->set_listener(nullptr);
            (*dw_it)->get_topic()->get_impl()->dereference();
            delete (*dw_it);
            vit->second.erase(dw_it);
            if (vit->second.empty())
            {
                writers_.erase(vit);
            }
            return ReturnCode_t::RETCODE_OK;
        }
    }
    return ReturnCode_t::RETCODE_ERROR;
}

DataWriter* PublisherImpl::lookup_datawriter(
        const std::string& topic_name) const
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    auto it = writers_.find(topic_name);
    if (it != writers_.end() && it->second.size() > 0)
    {
        return it->second.front()->user_datawriter_;
    }
    return nullptr;
}

bool PublisherImpl::get_datawriters(
        std::vector<DataWriter*>& writers) const
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto vit : writers_)
    {
        for (DataWriterImpl* dw : vit.second)
        {
            writers.push_back(dw->user_datawriter_);
        }
    }
    return true;
}

bool PublisherImpl::has_datawriters() const
{
    if (writers_.empty())
    {
        return false;
    }
    return true;
}

bool PublisherImpl::contains_entity(
        const fastrtps::rtps::InstanceHandle_t& handle) const
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto vit : writers_)
    {
        for (DataWriterImpl* dw : vit.second)
        {
            InstanceHandle_t h(dw->guid());
            if (h == handle)
            {
                return true;
            }
        }
    }
    return false;
}

/* TODO
   bool PublisherImpl::suspend_publications()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

/* TODO
   bool PublisherImpl::resume_publications()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

/* TODO
   bool PublisherImpl::begin_coherent_changes()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

/* TODO
   bool PublisherImpl::end_coherent_changes()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */


ReturnCode_t PublisherImpl::set_default_datawriter_qos(
        const DataWriterQos& qos)
{
    if (&qos == &DATAWRITER_QOS_DEFAULT)
    {
        DataWriterImpl::set_qos(default_datawriter_qos_, DATAWRITER_QOS_DEFAULT, true);
    }

    ReturnCode_t ret_val = DataWriterImpl::check_qos(qos);
    if (!ret_val)
    {
        return ret_val;
    }
    DataWriterImpl::set_qos(default_datawriter_qos_, qos, true);
    return ReturnCode_t::RETCODE_OK;
}

const DataWriterQos& PublisherImpl::get_default_datawriter_qos() const
{
    return default_datawriter_qos_;
}

/* TODO
   bool PublisherImpl::copy_from_topic_qos(
        fastrtps::WriterQos&,
        const fastrtps::TopicAttributes&) const
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

ReturnCode_t PublisherImpl::wait_for_acknowledgments(
        const Duration_t& max_wait)
{
    Duration_t current = max_wait;
    Duration_t begin, end;
    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto& vit : writers_)
    {
        for (DataWriterImpl* dw : vit.second)
        {
            participant_->get_current_time(begin);
            if (!dw->wait_for_acknowledgments(current))
            {
                return ReturnCode_t::RETCODE_ERROR;
            }
            // Check ellapsed time and decrement
            participant_->get_current_time(end);
            current = current - (end - begin);
            if (current < fastrtps::c_TimeZero)
            {
                return ReturnCode_t::RETCODE_TIMEOUT;
            }
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

const DomainParticipant* PublisherImpl::get_participant() const
{
    return participant_->get_participant();
}

const Publisher* PublisherImpl::get_publisher() const
{
    return user_publisher_;
}

/* TODO
   bool PublisherImpl::delete_contained_entities()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

const InstanceHandle_t& PublisherImpl::get_instance_handle() const
{
    return handle_;
}

bool PublisherImpl::type_in_use(
        const std::string& type_name) const
{
    for (auto it : writers_)
    {
        for (DataWriterImpl* writer : it.second)
        {
            if (writer->get_topic()->get_type_name() == type_name)
            {
                return true; // Is in use
            }
        }
    }
    return false;
}

void PublisherImpl::set_qos(
        PublisherQos& to,
        const PublisherQos& from,
        bool first_time)
{
    if (first_time && !(to.presentation() == from.presentation()))
    {
        to.presentation(from.presentation());
        to.presentation().hasChanged = true;
    }
    if (!(to.partition() == from.partition()))
    {
        to.partition() = from.partition();
        to.partition().hasChanged = true;
    }
    if (!(to.group_data() == from.group_data()))
    {
        to.group_data() = from.group_data();
        to.group_data().hasChanged = true;
    }
    if (!(to.entity_factory() == from.entity_factory()))
    {
        to.entity_factory() = from.entity_factory();
    }
}

ReturnCode_t PublisherImpl::check_qos(
        const PublisherQos& qos)
{
    (void) qos;
    return ReturnCode_t::RETCODE_OK;
}

bool PublisherImpl::can_qos_be_updated(
        const PublisherQos& to,
        const PublisherQos& from)
{
    (void) to;
    (void) from;
    return true;
}

} // dds
} // fastdds
} // eprosima
