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

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/dds/log/Log.hpp>

#include <functional>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {

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
    if (!qos.check_qos())
    {
        if (!qos_.can_qos_be_updated(qos))
        {
            return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
        }
        qos_.set_qos(qos, false);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
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
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_publication_matched(publisher_->user_publisher_, info);
    }
}

void PublisherImpl::PublisherWriterListener::on_liveliness_lost(
        DataWriter* /*writer*/,
        const LivelinessLostStatus& status)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_liveliness_lost(publisher_->user_publisher_, status);
    }
}

void PublisherImpl::PublisherWriterListener::on_offered_deadline_missed(
        DataWriter* /*writer*/,
        const fastrtps::OfferedDeadlineMissedStatus& status)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_offered_deadline_missed(publisher_->user_publisher_, status);
    }
}

DataWriter* PublisherImpl::create_datawriter(
        const fastrtps::TopicAttributes& topic_att,
        const fastrtps::WriterQos& writer_qos,
        DataWriterListener* listener)
{
    logInfo(PUBLISHER, "CREATING WRITER IN TOPIC: " << topic_att.getTopicName());
    //Look for the correct type registration
    TypeSupport type_support = participant_->find_type(topic_att.getTopicDataType().to_string());

    /// Preconditions
    // Check the type was registered.
    if (type_support.empty())
    {
        logError(PUBLISHER, "Type: " << topic_att.getTopicDataType() << " Not Registered");
        return nullptr;
    }

    // Check the type supports keys.
    if (topic_att.topicKind == WITH_KEY && !type_support->m_isGetKeyDefined)
    {
        logError(PUBLISHER, "Keyed Topic " << topic_att.getTopicName() << " needs getKey function");
        return nullptr;
    }

    if (!topic_att.checkQos() || !writer_qos.checkQos())
    {
        return nullptr;
    }

    //TODO: Fix when PublisherAttributes are correct decomposed in qos
    WriterAttributes w_att;
    //w_att.throughputController = qos_.publisher_attr.throughputController;
    w_att.endpoint.durabilityKind = writer_qos.m_durability.durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    //w_att.endpoint.multicastLocatorList = qos_.publisher_attr.multicastLocatorList;
    w_att.endpoint.reliabilityKind = writer_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = topic_att.topicKind;
    //w_att.endpoint.unicastLocatorList = qos_.publisher_attr.unicastLocatorList;
    //w_att.endpoint.remoteLocatorList = qos_.publisher_attr.remoteLocatorList;
    w_att.mode = writer_qos.m_publishMode.kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
    //w_att.endpoint.properties = qos_.publisher_attr.properties;

    //    if (qos_.publisher_attr.getEntityID() > 0)
    //    {
    //        w_att.endpoint.setEntityID(static_cast<uint8_t>(qos_.publisher_attr.getEntityID()));
    //    }

    //    if (qos_.publisher_attr.getUserDefinedID() > 0)
    //    {
    //        w_att.endpoint.setUserDefinedID(static_cast<uint8_t>(qos_.publisher_attr.getUserDefinedID()));
    //    }

    //w_att.times = qos_.publisher_attr.times;
    w_att.liveliness_kind = writer_qos.m_liveliness.kind;
    w_att.liveliness_lease_duration = writer_qos.m_liveliness.lease_duration;
    //w_att.matched_readers_allocation = qos_.publisher_attr.matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_att.getTopicName().c_str());
    w_att.endpoint.properties.properties().push_back(std::move(property));

    if (writer_qos.m_partition.names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for (auto partition : writer_qos.m_partition.names())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        w_att.endpoint.properties.properties().push_back(std::move(property));
    }

    if (writer_qos.m_disablePositiveACKs.enabled &&
            writer_qos.m_disablePositiveACKs.duration != c_TimeInfinite)
    {
        w_att.disable_positive_acks = true;
        w_att.keep_duration = writer_qos.m_disablePositiveACKs.duration;
    }

    DataWriterImpl* impl = new DataWriterImpl(
        this,
        type_support,
        topic_att,
        w_att,
        writer_qos,
        //qos_.publisher_attr.historyMemoryPolicy,
        fastrtps::rtps::MemoryManagementPolicy_t(),
        listener);

    if (impl->writer_ == nullptr)
    {
        logError(PUBLISHER, "Problem creating associated Writer");
        delete impl;
        return nullptr;
    }

    DataWriter* writer = new DataWriter(impl);
    impl->user_datawriter_ = writer;

    //REGISTER THE WRITER
    rtps_participant_->registerWriter(impl->writer_, topic_att, writer_qos);

    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        writers_[topic_att.getTopicName().to_string()].push_back(impl);
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
    auto vit = writers_.find(writer->get_topic().getTopicName().to_string());
    if (vit != writers_.end())
    {
        auto dw_it = std::find(vit->second.begin(), vit->second.end(), writer->impl_);
        if (dw_it != vit->second.end())
        {
            (*dw_it)->set_listener(nullptr);
            vit->second.erase(dw_it);
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
    if (!qos.check_qos())
    {
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    if (!(default_datawriter_qos_.durability() == qos.durability()))
    {
        default_datawriter_qos_.durability() = qos.durability();
        default_datawriter_qos_.durability().hasChanged = true;
    }
    if (!(default_datawriter_qos_.durability_service() == qos.durability_service()))
    {
        default_datawriter_qos_.durability_service() = qos.durability_service();
        default_datawriter_qos_.durability_service().hasChanged = true;
    }
    if (!(default_datawriter_qos_.deadline() == qos.deadline()))
    {
        default_datawriter_qos_.deadline() = qos.deadline();
        default_datawriter_qos_.deadline().hasChanged = true;
    }
    if (!(default_datawriter_qos_.latency_budget() == qos.latency_budget()))
    {
        default_datawriter_qos_.latency_budget() = qos.latency_budget();
        default_datawriter_qos_.latency_budget().hasChanged = true;
    }
    if (!(default_datawriter_qos_.liveliness() == qos.liveliness()))
    {
        default_datawriter_qos_.liveliness() = qos.liveliness();
        default_datawriter_qos_.liveliness().hasChanged = true;
    }
    if (!(default_datawriter_qos_.reliability() == qos.reliability()))
    {
        default_datawriter_qos_.reliability() = qos.reliability();
        default_datawriter_qos_.reliability().hasChanged = true;
    }
    if (!(default_datawriter_qos_.destination_order() == qos.destination_order()))
    {
        default_datawriter_qos_.destination_order() = qos.destination_order();
        default_datawriter_qos_.destination_order().hasChanged = true;
    }
    if (!(default_datawriter_qos_.history() == qos.history()))
    {
        default_datawriter_qos_.history() = qos.history();
        default_datawriter_qos_.history().hasChanged = true;
    }
    if (!(default_datawriter_qos_.resource_limits() == qos.resource_limits()))
    {
        default_datawriter_qos_.resource_limits() = qos.resource_limits();
        default_datawriter_qos_.resource_limits().hasChanged = true;
    }
    if (!(default_datawriter_qos_.transport_priority() == qos.transport_priority()))
    {
        default_datawriter_qos_.transport_priority() = qos.transport_priority();
        default_datawriter_qos_.transport_priority().hasChanged = true;
    }
    if (!(default_datawriter_qos_.lifespan() == qos.lifespan()))
    {
        default_datawriter_qos_.lifespan() = qos.lifespan();
        default_datawriter_qos_.lifespan().hasChanged = true;
    }
    if (!(default_datawriter_qos_.user_data() == qos.user_data()))
    {
        default_datawriter_qos_.user_data() = qos.user_data();
        default_datawriter_qos_.user_data().hasChanged = true;
    }
    if (!(default_datawriter_qos_.ownership() == qos.ownership()))
    {
        default_datawriter_qos_.ownership() = qos.ownership();
        default_datawriter_qos_.ownership().hasChanged = true;
    }
    if (!(default_datawriter_qos_.ownership_strength() == qos.ownership_strength()))
    {
        default_datawriter_qos_.ownership_strength() = qos.ownership_strength();
        default_datawriter_qos_.ownership_strength().hasChanged = true;
    }
    if (!(default_datawriter_qos_.writer_data_lifecycle() == qos.writer_data_lifecycle()))
    {
        default_datawriter_qos_.writer_data_lifecycle() = qos.writer_data_lifecycle();
    }
    if (!(default_datawriter_qos_.publish_mode() == qos.publish_mode()))
    {
        default_datawriter_qos_.publish_mode() = qos.publish_mode();
    }
    if (!(default_datawriter_qos_.representation() == qos.representation()))
    {
        default_datawriter_qos_.representation() = qos.representation();
        default_datawriter_qos_.representation().hasChanged = true;
    }
    if (!(default_datawriter_qos_.properties() == qos.properties()))
    {
        default_datawriter_qos_.properties() = qos.properties();
    }
    if (!(default_datawriter_qos_.reliable_writer_data() == qos.reliable_writer_data()))
    {
        default_datawriter_qos_.reliable_writer_data() = qos.reliable_writer_data();
    }
    if (!(default_datawriter_qos_.endpoint_data() == qos.endpoint_data()))
    {
        default_datawriter_qos_.endpoint_data() = qos.endpoint_data();
    }
    if (!(default_datawriter_qos_.writer_resources() == qos.writer_resources()))
    {
        default_datawriter_qos_.writer_resources() = qos.writer_resources();
    }
    if (!(default_datawriter_qos_.throughput_controller() == qos.throughput_controller()))
    {
        default_datawriter_qos_.throughput_controller() = qos.throughput_controller();
    }
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
            if (current < c_TimeZero)
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
            if (writer->get_topic().getTopicDataType() == type_name)
            {
                return true; // Is in use
            }
        }
    }
    return false;
}

} // dds
} // fastdds
} // eprosima
