// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * Publisher.cpp
 *
 */

#include "PublisherImpl.h"
#include "../participant/ParticipantImpl.h"
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/publisher/PublisherListener.h>

#include <fastrtps/publisher/PublisherHistory.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <functional>

using namespace eprosima::fastrtps;
using namespace ::rtps;
using namespace std::chrono;

using namespace std::chrono;

using namespace std::chrono;

PublisherImpl::PublisherImpl(
        ParticipantImpl* p,
        const PublisherAttributes& att,
        PublisherListener* listen )
    : mp_participant(p)
    , m_att(att)
#pragma warning (disable : 4355 )
    , mp_listener(listen)
#pragma warning (disable : 4355 )
    , m_writerListener(this)
    , mp_userPublisher(nullptr)
    , mp_rtpsParticipant(nullptr)
    , deadline_duration_us_(m_att.qos.m_deadline.period.to_ns() * 1e-3)
    , lifespan_duration_us_(m_att.qos.m_lifespan.duration.to_ns() * 1e-3)
{
}

PublisherImpl::~PublisherImpl()
{
    delete(this->mp_userPublisher);
}

DataWriter* PublisherImpl::create_writer(
        const TopicAttributes& topic_att,
        const WriterQos& wqos,
        DataWriterListener* listener)
{
    logInfo(PUBLISHER, "CREATING WRITER IN TOPIC: " << topic_att.getTopicName());
    //Look for the correct type registration

    TopicDataType* topic_data_type = nullptr;
    mp_participant->getRegisteredType(topic_att.topicDataType.c_str(), &topic_data_type);

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

    if(!topic_att.checkQos())
    {
        return nullptr;
    }

    WriterAttributes watt;
    watt.throughputController = m_att.throughputController;
    watt.endpoint.durabilityKind = m_att.qos.m_durability.durabilityKind();
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.multicastLocatorList = m_att.multicastLocatorList;
    watt.endpoint.reliabilityKind = m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    watt.endpoint.topicKind = topic_att.topicKind;
    watt.endpoint.unicastLocatorList = m_att.unicastLocatorList;
    watt.endpoint.remoteLocatorList = m_att.remoteLocatorList;
    watt.mode = m_att.qos.m_publishMode.kind == eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE
                    ? SYNCHRONOUS_WRITER
                    : ASYNCHRONOUS_WRITER;
    watt.endpoint.properties = m_att.properties;
    if(m_att.getEntityID()>0)
    {
        watt.endpoint.setEntityID(static_cast<uint8_t>(m_att.getEntityID()));
    }
    if(m_att.getUserDefinedID()>0)
    {
        watt.endpoint.setUserDefinedID(static_cast<uint8_t>(m_att.getUserDefinedID()));
    }
    watt.times = m_att.times;
    watt.matched_readers_allocation = m_att.matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_att.getTopicName().c_str());
    watt.endpoint.properties.properties().push_back(std::move(property));
    if(m_att.qos.m_partition.getNames().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for(auto partition : m_att.qos.m_partition.getNames())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        watt.endpoint.properties.properties().push_back(std::move(property));
    }
    if (m_att.qos.m_disablePositiveACKs.enabled &&
            m_att.qos.m_disablePositiveACKs.duration != c_TimeInfinite)
    {
        watt.disable_positive_acks = true;
        watt.keep_duration = m_att.qos.m_disablePositiveACKs.duration;
    }

    PublisherHistory history(this, topic_data_type->m_typeSize
#if HAVE_SECURITY
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            + 20 /*SecureDataHeader*/ + 4 + ((2* 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/
#endif
             , topic_att.historyQos, topic_att.resourceLimitsQos, m_att.historyMemoryPolicy);

    DataWriter* writer = new DataWriter(
        this,
        topic_data_type,
        topic_att,
        watt,
        wqos,
        std::move(history),
        listener);

    if(writer->writer_ == nullptr)
    {
        logError(PUBLISHER, "Problem creating associated Writer");
        delete writer;
        return nullptr;
    }

    //REGISTER THE WRITER
    mp_rtpsParticipant->registerWriter(writer->writer_, topic_att, m_att.qos);

    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        writers_[topic_att.getTopicDataType().to_string()] = writer;
    }

    return writer;
}

bool PublisherImpl::update_writer(
        DataWriter* writer,
        const TopicAttributes& topic_att,
        const WriterQos& qos)
{
    bool result = mp_rtpsParticipant->updateWriter(writer->writer_, topic_att, qos);

    const TopicAttributes& old_topic_att = writer->topic_att_;

    if (result && old_topic_att.getTopicDataType() != topic_att.getTopicDataType())
    {
        std::lock_guard<std::mutex> lock(mtx_writers_);
        auto it = writers_.find(old_topic_att.getTopicDataType().to_string());
        assert(it != writers_.end());
        writers_.erase(it);
        writers_[topic_att.getTopicDataType().to_string()] = writer;
    }

    return result;
}

bool PublisherImpl::updateAttributes(const PublisherAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    {
        if(att.unicastLocatorList.size() != this->m_att.unicastLocatorList.size() ||
                att.multicastLocatorList.size() != this->m_att.multicastLocatorList.size())
        {
            logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
            updated &= false;
        }
        else
        {
            for(LocatorListConstIterator lit1 = this->m_att.unicastLocatorList.begin();
                    lit1!=this->m_att.unicastLocatorList.end();++lit1)
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
            for(LocatorListConstIterator lit1 = this->m_att.multicastLocatorList.begin();
                    lit1!=this->m_att.multicastLocatorList.end();++lit1)
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
        this->m_att = att;
    }

    return updated;
}

void PublisherImpl::PublisherWriterListener::onWriterMatched(
        DataWriter* /*writer*/,
        MatchingInfo& info)
{
    if( mp_publisherImpl->mp_listener != nullptr )
    {
        mp_publisherImpl->mp_listener->onPublicationMatched(mp_publisherImpl->mp_userPublisher, info);
    }
}
