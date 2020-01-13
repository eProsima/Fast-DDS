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
#include <fastdds/topic/DataWriterImpl.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastrtps/log/Log.h>

#include <functional>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {

PublisherImpl::PublisherImpl(
        DomainParticipantImpl* p,
        const PublisherQos& qos,
        const fastrtps::PublisherAttributes& att,
        PublisherListener* listen)
    : participant_(p)
    , qos_(&qos == &PUBLISHER_QOS_DEFAULT ? participant_->get_default_publisher_qos() : qos)
    , att_(att)
    , listener_(listen)
    , publisher_listener_(this)
    , user_publisher_(nullptr)
    , rtps_participant_(p->rtps_participant())
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

PublisherListener* PublisherImpl::get_listener()
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
        const DataWriterQos& writer_qos,
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
    if (topic_att.topicKind == WITH_KEY && !type_support.get()->m_isGetKeyDefined)
    {
        logError(PUBLISHER, "Keyed Topic " << topic_att.getTopicName() << " needs getKey function");
        return nullptr;
    }

    if (!topic_att.checkQos() || !writer_qos.checkQos())
    {
        return nullptr;
    }

    WriterAttributes w_att;
    w_att.throughputController = att_.throughputController;
    w_att.endpoint.durabilityKind = writer_qos.durability.durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    w_att.endpoint.multicastLocatorList = att_.multicastLocatorList;
    w_att.endpoint.reliabilityKind = writer_qos.reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = topic_att.topicKind;
    w_att.endpoint.unicastLocatorList = att_.unicastLocatorList;
    w_att.endpoint.remoteLocatorList = att_.remoteLocatorList;
    w_att.mode = writer_qos.publish_mode.kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
    w_att.endpoint.properties = att_.properties;

    if (att_.getEntityID() > 0)
    {
        w_att.endpoint.setEntityID(static_cast<uint8_t>(att_.getEntityID()));
    }

    if (att_.getUserDefinedID() > 0)
    {
        w_att.endpoint.setUserDefinedID(static_cast<uint8_t>(att_.getUserDefinedID()));
    }

    w_att.times = att_.times;
    w_att.liveliness_kind = writer_qos.liveliness.kind;
    w_att.liveliness_lease_duration = writer_qos.liveliness.lease_duration;
    w_att.matched_readers_allocation = att_.matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_att.getTopicName().c_str());
    w_att.endpoint.properties.properties().push_back(std::move(property));

    if (qos_.partition.names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for (auto partition : qos_.partition.names())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        w_att.endpoint.properties.properties().push_back(std::move(property));
    }

    if (writer_qos.disable_positive_ACKs.enabled &&
            writer_qos.disable_positive_ACKs.duration != c_TimeInfinite)
    {
        w_att.disable_positive_acks = true;
        w_att.keep_duration = writer_qos.disable_positive_ACKs.duration;
    }

    DataWriterImpl* impl = new DataWriterImpl(
        this,
        type_support,
        topic_att,
        w_att,
        writer_qos,
        att_.historyMemoryPolicy,
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
    WriterQos wqos = writer_qos.changeToWriterQos();
    rtps_participant_->registerWriter(impl->writer_, topic_att, wqos);

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
    if (&qos == &DDS_DATAWRITER_QOS_DEFAULT)
    {
        default_datawriter_qos_.setQos(DDS_DATAWRITER_QOS_DEFAULT, true);
        return ReturnCode_t::RETCODE_OK;
    }
    else if (qos.checkQos())
    {
        default_datawriter_qos_.setQos(qos, true);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

const DataWriterQos& PublisherImpl::get_default_datawriter_qos() const
{
    return default_datawriter_qos_;
}

ReturnCode_t PublisherImpl::copy_from_topic_qos(
        DataWriterQos& reader_qos,
        const TopicQos& topic_qos) const
{
    reader_qos.copyFromTopicQos(topic_qos);
    return ReturnCode_t::RETCODE_OK;
}

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

const fastrtps::PublisherAttributes& PublisherImpl::get_attributes() const
{
    return att_;
}

bool PublisherImpl::set_attributes(
        const fastrtps::PublisherAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if (att_.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    {
        if (att.unicastLocatorList.size() != att_.unicastLocatorList.size() ||
                att.multicastLocatorList.size() != att_.multicastLocatorList.size())
        {
            logWarning(PUBLISHER, "Locator Lists cannot be changed or updated in this version");
            updated &= false;
        }
        else
        {
            for (LocatorListConstIterator lit1 = att_.unicastLocatorList.begin();
                    lit1 != att_.unicastLocatorList.end(); ++lit1)
            {
                missing = true;
                for (LocatorListConstIterator lit2 = att.unicastLocatorList.begin();
                        lit2 != att.unicastLocatorList.end(); ++lit2)
                {
                    if (*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if (missing)
                {
                    logWarning(PUBLISHER, "Locator: " << *lit1 << " not present in new list");
                    logWarning(PUBLISHER, "Locator Lists cannot be changed or updated in this version");
                }
            }
            for (LocatorListConstIterator lit1 = att_.multicastLocatorList.begin();
                    lit1 != att_.multicastLocatorList.end(); ++lit1)
            {
                missing = true;
                for (LocatorListConstIterator lit2 = att.multicastLocatorList.begin();
                        lit2 != att.multicastLocatorList.end(); ++lit2)
                {
                    if (*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if (missing)
                {
                    logWarning(PUBLISHER, "Locator: " << *lit1 << " not present in new list");
                    logWarning(PUBLISHER, "Locator Lists cannot be changed or updated in this version");
                }
            }
        }
    }

    if (updated)
    {
        att_ = att;
    }

    return updated;
}

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
