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

#include "PublisherImpl.hpp"
#include "../domain/ParticipantImpl.hpp"

#include <fastdds/publisher/Publisher.hpp>
#include <fastdds/publisher/PublisherListener.hpp>
#include <fastdds/topic/DataWriter.hpp>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/log/Log.h>

#include <functional>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {

PublisherImpl::PublisherImpl(
        ParticipantImpl* p,
        const PublisherQos& qos,
        const fastrtps::PublisherAttributes& att,
        PublisherListener* listen)
    : participant_(p)
    , qos_(qos)
    , att_(att)
    , listener_(listen)
    , publisher_listener_(this)
    , user_publisher_(nullptr)
    , rtps_participant_(p->rtps_participant())
{
}

PublisherImpl::~PublisherImpl()
{
    for (auto& writer_it : writers_)
    {
        delete_datawriter(writer_it.second);
        delete writer_it.second;
    }

    delete user_publisher_;
}

const PublisherQos& PublisherImpl::get_qos() const
{
    return qos_;
}

bool PublisherImpl::set_qos(
        const PublisherQos& qos)
{
    if(qos_.canQosBeUpdated(qos))
    {
        qos_.setQos(qos, false);
        return true;
    }
    return false;
}

const PublisherListener* PublisherImpl::get_listener() const
{
    return listener_;
}

bool PublisherImpl::set_listener(
        PublisherListener* listener)
{
    if (listener_ == listener)
    {
        return false;
    }
    listener_ = listener;
    return true;
}

void PublisherImpl::PublisherWriterListener::on_publication_matched(
        DataWriter* /*writer*/,
        MatchingInfo& info)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_publication_matched(publisher_->user_publisher_, info);
    }
}

void PublisherImpl::PublisherWriterListener::on_liveliness_lost(
        DataWriter* /*writer*/,
        const fastrtps::LivelinessLostStatus &status)
{
    if (publisher_->listener_ != nullptr)
    {
        publisher_->listener_->on_liveliness_lost(publisher_->user_publisher_, status);
    }
}

void PublisherImpl::PublisherWriterListener::on_offered_deadline_missed(
        DataWriter* /*writer*/,
        const fastrtps::OfferedDeadlineMissedStatus &status)
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
    TopicDataType* topic_data_type = participant_->find_type(topic_att.getTopicDataType().to_string());

    /// Preconditions
    // Check the type was registered.
    if(topic_data_type == nullptr)
    {
        logError(PUBLISHER, "Type: "<< topic_att.getTopicDataType() << " Not Registered");
        return nullptr;
    }

    // Check the type supports keys.
    if(topic_att.topicKind == WITH_KEY && !topic_data_type->m_isGetKeyDefined)
    {
        logError(PUBLISHER, "Keyed Topic " << topic_att.getTopicName() << " needs getKey function");
        return nullptr;
    }

    if(!topic_att.checkQos() || !writer_qos.checkQos())
    {
        return nullptr;
    }

    WriterAttributes w_att;
    w_att.throughputController = att_.throughputController;
    w_att.endpoint.durabilityKind = writer_qos.m_durability.durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    w_att.endpoint.multicastLocatorList = att_.multicastLocatorList;
    w_att.endpoint.reliabilityKind = writer_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = topic_att.topicKind;
    w_att.endpoint.unicastLocatorList = att_.unicastLocatorList;
    w_att.endpoint.remoteLocatorList = att_.remoteLocatorList;
    w_att.mode = writer_qos.m_publishMode.kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
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
    w_att.matched_readers_allocation = att_.matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_att.getTopicName().c_str());
    w_att.endpoint.properties.properties().push_back(std::move(property));

    if(writer_qos.m_partition.getNames().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for(auto partition : writer_qos.m_partition.getNames())
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

    PublisherHistory history(
        att_, topic_data_type, topic_data_type->m_typeSize
#if HAVE_SECURITY
        // In future v2 changepool is in writer, and writer set this value to cachechagepool.
        + 20 /*SecureDataHeader*/ + 4 + ((2* 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
        + 16 + 4 /*SecureDataTag*/
#endif
        , topic_att.historyQos, topic_att.resourceLimitsQos, att_.historyMemoryPolicy);

    DataWriter* writer = new DataWriter(
        this,
        topic_data_type,
        topic_att,
        w_att,
        writer_qos,
        std::move(history),
        listener);

    if(writer->writer_ == nullptr)
    {
        logError(PUBLISHER, "Problem creating associated Writer");
        delete writer;
        return nullptr;
    }

    //REGISTER THE WRITER
    rtps_participant_->registerWriter(writer->writer_, topic_att, writer_qos);

    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        writers_[topic_att.getTopicDataType().to_string()] = writer;
    }

    return writer;
}

bool PublisherImpl::delete_datawriter(
        DataWriter* writer)
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    auto it = writers_.find(writer->get_topic().getTopicName().to_string());
    if (it != writers_.end() && it->second == writer)
    {
        writers_.erase(it);
        return true;
    }
    return false;
}

DataWriter* PublisherImpl::lookup_datawriter(
    const std::string& topic_name) const
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    auto it = writers_.find(topic_name);
    if (it != writers_.end())
    {
        return it->second;
    }
    return nullptr;
}

bool PublisherImpl::get_datawriters(
        std::vector<DataWriter*>& writers) const
{
    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto it : writers_)
    {
        writers.push_back(it.second);
    }
    return true;
}

bool PublisherImpl::suspend_publications()
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}

bool PublisherImpl::resume_publications()
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}

bool PublisherImpl::begin_coherent_changes()
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}

bool PublisherImpl::end_coherent_changes()
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}


bool PublisherImpl::set_default_datawriter_qos(
        const fastrtps::WriterQos& qos)
{
    if (default_datawriter_qos_.canQosBeUpdated(qos) && qos.checkQos())
    {
        default_datawriter_qos_.setQos(qos, false);
        return true;
    }
    return false;
}

const fastrtps::WriterQos& PublisherImpl::get_default_datawriter_qos() const
{
    return default_datawriter_qos_;
}

bool PublisherImpl::copy_from_topic_qos(
        fastrtps::WriterQos&,
        const fastrtps::TopicAttributes&) const
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}

bool PublisherImpl::wait_for_acknowledments(
        const Duration_t& max_wait)
{
    Duration_t current = max_wait;
    Duration_t begin, end;
    std::lock_guard<std::mutex> lock(mtx_writers_);
    for (auto& it : writers_)
    {
        participant_->get_current_time(begin);
        if (!it.second->wait_for_acknowledgments(current))
        {
            return false;
        }
        // Check ellapsed time and decrement
        participant_->get_current_time(end);
        current = current - (end - begin);
        if (current < c_TimeZero)
        {
            return false;
        }
    }
    return true;
}

const Participant* PublisherImpl::get_participant() const
{
    return participant_->get_participant();
}

const Publisher* PublisherImpl::get_publisher() const
{
    return user_publisher_;
}

bool PublisherImpl::delete_contained_entities()
{
    logError(PUBLISHER, "Operation not implemented");
    return false;
}

const fastrtps::PublisherAttributes& PublisherImpl::get_attributes() const
{
    return att_;
}

bool PublisherImpl::set_attributes(
        const fastrtps::PublisherAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if(att_.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    {
        if(att.unicastLocatorList.size() != att_.unicastLocatorList.size() ||
            att.multicastLocatorList.size() != att_.multicastLocatorList.size())
        {
            logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
            updated &= false;
        }
        else
        {
            for(LocatorListConstIterator lit1 = att_.unicastLocatorList.begin();
                 lit1!=att_.unicastLocatorList.end();++lit1)
            {
                missing = true;
                for(LocatorListConstIterator lit2 = att.unicastLocatorList.begin();
                     lit2!= att.unicastLocatorList.end();++lit2)
                {
                    if(*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if(missing)
                {
                    logWarning(PUBLISHER,"Locator: "<< *lit1 << " not present in new list");
                    logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
                }
            }
            for(LocatorListConstIterator lit1 = att_.multicastLocatorList.begin();
                 lit1!=att_.multicastLocatorList.end();++lit1)
            {
                missing = true;
                for(LocatorListConstIterator lit2 = att.multicastLocatorList.begin();
                     lit2!= att.multicastLocatorList.end();++lit2)
                {
                    if(*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if(missing)
                {
                    logWarning(PUBLISHER,"Locator: "<< *lit1<< " not present in new list");
                    logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
                }
            }
        }
    }

    if(updated)
    {
        att_ = att;
    }

    return updated;
}

const InstanceHandle_t& PublisherImpl::get_instance_handle() const
{
    return handle_;
}

} // fastdds
} // eprosima
