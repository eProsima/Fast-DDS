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

/**
 * @file SubscriberImpl.cpp
 *
 */

#include "SubscriberImpl.h"
#include "../participant/ParticipantImpl.h"
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps::rtps;


namespace eprosima {
namespace fastrtps {


SubscriberImpl::SubscriberImpl(
        ParticipantImpl* p,
        TopicDataType* ptype,
        const SubscriberAttributes& att,
        SubscriberListener* listen)
    : mp_participant(p)
    , mp_reader(nullptr)
    , mp_type(ptype)
    , m_att(att)
#pragma warning (disable : 4355 )
    , m_history(this,ptype->m_typeSize  + 3/*Possible alignment*/, att.topic.historyQos, att.topic.resourceLimitsQos,att.historyMemoryPolicy)
    , mp_listener(listen)
    , m_readerListener(this)
    , mp_userSubscriber(nullptr)
    , mp_rtpsParticipant(nullptr)
    , deadline_timer_(std::bind(&SubscriberImpl::check_deadlines, this),
                      att.qos.m_deadline.period,
                      mp_participant->get_resource_event().getIOService(),
                      mp_participant->get_resource_event().getThread())
    , deadline_duration_(att.qos.m_deadline.period)
    {
    }


SubscriberImpl::~SubscriberImpl()
{
    if(mp_reader != nullptr)
    {
        logInfo(SUBSCRIBER,this->getGuid().entityId << " in topic: "<<this->m_att.topic.topicName);
    }

    RTPSDomain::removeRTPSReader(mp_reader);
    delete(this->mp_userSubscriber);
}


void SubscriberImpl::waitForUnreadMessage()
{
    if(m_history.getUnreadCount()==0)
    {
        do
        {
            m_history.waitSemaphore();
        }
        while(m_history.getUnreadCount() == 0);
    }
}



bool SubscriberImpl::readNextData(void* data,SampleInfo_t* info)
{
    return this->m_history.readNextData(data,info);
}

bool SubscriberImpl::takeNextData(void* data,SampleInfo_t* info) {
    return this->m_history.takeNextData(data,info);
}



const GUID_t& SubscriberImpl::getGuid(){
    return mp_reader->getGuid();
}



bool SubscriberImpl::updateAttributes(const SubscriberAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if(att.unicastLocatorList.size() != this->m_att.unicastLocatorList.size() ||
            att.multicastLocatorList.size() != this->m_att.multicastLocatorList.size())
    {
        logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
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
                logWarning(RTPS_READER,"Locator: "<< *lit1 << " not present in new list");
                logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
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
                logWarning(RTPS_READER,"Locator: "<< *lit1<< " not present in new list");
                logWarning(RTPS_READER,"Locator Lists cannot be changed or updated in this version");
            }
        }
    }

    //TOPIC ATTRIBUTES
    if(this->m_att.topic != att.topic)
    {
        logWarning(RTPS_READER,"Topic Attributes cannot be updated");
        updated &= false;
    }
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if(!this->m_att.qos.canQosBeUpdated(att.qos))
    {
        updated &=false;
    }
    if(updated)
    {
        this->m_att.expectsInlineQos = att.expectsInlineQos;
        if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulReader* sfr = (StatefulReader*)mp_reader;
            sfr->updateTimes(att.times);
        }
        this->m_att.qos.setQos(att.qos,false);
        //NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
        mp_rtpsParticipant->updateReader(this->mp_reader, m_att.topic, m_att.qos);
    }
    return updated;
}

void SubscriberImpl::SubscriberReaderListener::onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const /*change*/)
{
    mp_subscriberImpl->onNewCacheChangeAdded();

    if(mp_subscriberImpl->mp_listener != nullptr)
    {
        mp_subscriberImpl->mp_listener->onNewDataMessage(mp_subscriberImpl->mp_userSubscriber);
    }
}

void SubscriberImpl::SubscriberReaderListener::onReaderMatched(RTPSReader* /*reader*/, MatchingInfo& info)
{
    if (this->mp_subscriberImpl->mp_listener != nullptr)
    {
        mp_subscriberImpl->mp_listener->onSubscriptionMatched(mp_subscriberImpl->mp_userSubscriber,info);
    }
}

void SubscriberImpl::onNewCacheChangeAdded()
{
    if (m_att.qos.m_deadline.period != rtps::c_TimeInfinite)
    {
        deadline_timer_.restart_timer();
    }
}

/*!
 * @brief Returns there is a clean state with all Publishers.
 * It occurs when the Subscriber received all samples sent by Publishers. In other words,
 * its WriterProxies are up to date.
 * @return There is a clean state with all Publishers.
 */
bool SubscriberImpl::isInCleanState() const
{
    return mp_reader->isInCleanState();
}

uint64_t SubscriberImpl::getUnreadCount() const
{
    return m_history.getUnreadCount();
}

void SubscriberImpl::check_deadlines()
{
    assert(m_att.qos.m_deadline.period != rtps::c_TimeInfinite);

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Get the latest samples from the history
    std::vector<CacheChange_t *> samples;
    m_history.get_latest_samples(samples);

    if (samples.empty())
    {
        return;
    }

    // Time of the earliest sample among all topic instances
    Time_t minTime = samples.front()->sourceTimestamp;

    // Check if any instance missed the dealine
    for (const auto &sample : samples)
    {
        if (sample->sourceTimestamp < minTime)
        {
            minTime = sample->sourceTimestamp;
        }

        if (now - sample->sourceTimestamp > deadline_duration_)
        {
            mp_listener->on_requested_deadline_missed(sample->instanceHandle);
        }
    }

    // Restart the timer
    Duration_t interval = deadline_duration_ - now + minTime > 0? deadline_duration_ - now + minTime: deadline_duration_;
    deadline_timer_.update_interval(interval);
    deadline_timer_.restart_timer();
}

} /* namespace fastrtps */
} /* namespace eprosima */
