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
#include <fastrtps/TopicDataType.h>
#include <fastrtps/publisher/PublisherListener.h>

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
        TopicDataType* pdatatype,
        const PublisherAttributes& att,
        PublisherListener* listen )
    : mp_participant(p)
    , mp_writer(nullptr)
    , mp_type(pdatatype)
    , m_att(att)
#pragma warning (disable : 4355 )
    , m_history(this, pdatatype->m_typeSize
#if HAVE_SECURITY
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            + 20 /*SecureDataHeader*/ + 4 + ((2* 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/
#endif
            , att.topic.historyQos, att.topic.resourceLimitsQos, att.historyMemoryPolicy)
    , mp_listener(listen)
#pragma warning (disable : 4355 )
    , m_writerListener(this)
    , mp_userPublisher(nullptr)
    , mp_rtpsParticipant(nullptr)
    , high_mark_for_frag_(0)
    , deadline_timer_(std::bind(&PublisherImpl::deadline_missed, this),
                      att.qos.m_deadline.period.to_ns() * 1e-6,
                      mp_participant->get_resource_event().getIOService(),
                      mp_participant->get_resource_event().getThread())
    , deadline_duration_us_(m_att.qos.m_deadline.period.to_ns() * 1e-3)
    , timer_owner_()
    , deadline_missed_status_()
    , lifespan_timer_(std::bind(&PublisherImpl::lifespan_expired, this),
                      m_att.qos.m_lifespan.duration.to_ns() * 1e-6,
                      mp_participant->get_resource_event().getIOService(),
                      mp_participant->get_resource_event().getThread())
    , lifespan_duration_us_(m_att.qos.m_lifespan.duration.to_ns() * 1e-3)
{
}

PublisherImpl::~PublisherImpl()
{
    if(mp_writer != nullptr)
    {
        logInfo(PUBLISHER, this->getGuid().entityId << " in topic: " << this->m_att.topic.topicName);
    }

    RTPSDomain::removeRTPSWriter(mp_writer);
    delete(this->mp_userPublisher);
}



bool PublisherImpl::create_new_change(
        ChangeKind_t changeKind,
        void* data)
{
    WriteParams wparams;
    return create_new_change_with_params(changeKind, data, wparams);
}

bool PublisherImpl::create_new_change_with_params(
        ChangeKind_t changeKind,
        void* data,
        WriteParams& wparams)
{

    /// Preconditions
    if (data == nullptr)
    {
        logError(PUBLISHER, "Data pointer not valid");
        return false;
    }

    if(changeKind == NOT_ALIVE_UNREGISTERED || changeKind == NOT_ALIVE_DISPOSED ||
            changeKind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        if(m_att.topic.topicKind == NO_KEY)
        {
            logError(PUBLISHER,"Topic is NO_KEY, operation not permitted");
            return false;
        }
    }

    InstanceHandle_t handle;
    if(m_att.topic.topicKind == WITH_KEY)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = mp_writer->getAttributes().security_attributes().is_key_protected;
#endif
        mp_type->getKey(data,&handle,is_key_protected);
    }

    // Block lowlevel writer
    auto max_blocking_time = std::chrono::steady_clock::now() +
        std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(m_att.qos.m_reliability.max_blocking_time));
    std::unique_lock<std::recursive_timed_mutex> lock(mp_writer->getMutex(), std::defer_lock);

    if(lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* ch = mp_writer->new_change(mp_type->getSerializedSizeProvider(data), changeKind, handle);
        if(ch != nullptr)
        {
            if(changeKind == ALIVE)
            {
                //If these two checks are correct, we asume the cachechange is valid and thwn we can write to it.
                if(!mp_type->serialize(data, &ch->serializedPayload))
                {
                    logWarning(RTPS_WRITER,"RTPSWriter:Serialization returns false";);
                    m_history.release_Cache(ch);
                    return false;
                }
            }

            //TODO(Ricardo) This logic in a class. Then a user of rtps layer can use it.
            if(high_mark_for_frag_ == 0)
            {
                uint32_t max_data_size = mp_writer->getMaxDataSize();
                uint32_t writer_throughput_controller_bytes =
                    mp_writer->calculateMaxDataSize(m_att.throughputController.bytesPerPeriod);
                uint32_t participant_throughput_controller_bytes =
                    mp_writer->calculateMaxDataSize(
                            mp_rtpsParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod);

                high_mark_for_frag_ =
                    max_data_size > writer_throughput_controller_bytes ?
                    writer_throughput_controller_bytes :
                    (max_data_size > participant_throughput_controller_bytes ?
                     participant_throughput_controller_bytes :
                     max_data_size);
            }

            uint32_t final_high_mark_for_frag = high_mark_for_frag_;

            // If needed inlineqos for related_sample_identity, then remove the inlinqos size from final fragment size.
            if(wparams.related_sample_identity() != SampleIdentity::unknown())
            {
                final_high_mark_for_frag -= 32;
            }

            // If it is big data, fragment it.
            if(ch->serializedPayload.length > final_high_mark_for_frag)
            {
                // Check ASYNCHRONOUS_PUBLISH_MODE is being used, but it is an error case.
                if( m_att.qos.m_publishMode.kind != ASYNCHRONOUS_PUBLISH_MODE)
                {
                    logError(PUBLISHER, "Data cannot be sent. It's serialized size is " <<
                            ch->serializedPayload.length << "' which exceeds the maximum payload size of '" <<
                            final_high_mark_for_frag << "' and therefore ASYNCHRONOUS_PUBLISH_MODE must be used.");
                    m_history.release_Cache(ch);
                    return false;
                }

                /// Fragment the data.
                // Set the fragment size to the cachechange.
                // Note: high_mark will always be a value that can be casted to uint16_t)
                ch->setFragmentSize((uint16_t)final_high_mark_for_frag);
            }

            if(!this->m_history.add_pub_change(ch, wparams, lock, max_blocking_time))
            {
                m_history.release_Cache(ch);
                return false;
            }

            if (m_att.qos.m_deadline.period != c_TimeInfinite)
            {
                if (!m_history.set_next_deadline(
                            ch->instanceHandle,
                            steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
                {
                    logError(PUBLISHER, "Could not set the next deadline in the history");
                }
                else
                {
                    if (timer_owner_ == handle || timer_owner_ == InstanceHandle_t())
                    {
                        deadline_timer_reschedule();
                    }
                }
            }

            if (m_att.qos.m_lifespan.duration != c_TimeInfinite)
            {
                lifespan_duration_us_ = std::chrono::duration<double, std::ratio<1, 1000000>>(m_att.qos.m_lifespan.duration.to_ns() * 1e-3);
                lifespan_timer_.update_interval_millisec(m_att.qos.m_lifespan.duration.to_ns() * 1e-6);
                lifespan_timer_.restart_timer();
            }

            return true;
        }
    }

    return false;
}


bool PublisherImpl::removeMinSeqChange()
{
    return m_history.removeMinChange();
}

bool PublisherImpl::removeAllChange(size_t* removed)
{
    return m_history.removeAllChange(removed);
}

const GUID_t& PublisherImpl::getGuid()
{
    return mp_writer->getGuid();
}
//
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

    //TOPIC ATTRIBUTES
    if(this->m_att.topic != att.topic)
    {
        logWarning(PUBLISHER,"Topic Attributes cannot be updated");
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
        if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulWriter* sfw = (StatefulWriter*)mp_writer;
            sfw->updateTimes(att.times);
        }

        this->m_att.qos.setQos(att.qos,false);
        this->m_att = att;
        //Notify the participant that a Writer has changed its QOS
        mp_rtpsParticipant->updateWriter(this->mp_writer, m_att.topic, m_att.qos);

        // Deadline

        if (m_att.qos.m_deadline.period != c_TimeInfinite)
        {
            deadline_duration_us_ =
                    duration<double, std::ratio<1, 1000000>>(m_att.qos.m_deadline.period.to_ns() * 1e-3);
            deadline_timer_.update_interval_millisec(m_att.qos.m_deadline.period.to_ns() * 1e-6);
        }
        else
        {
            deadline_timer_.cancel_timer();
        }

        // Lifespan

        if (m_att.qos.m_lifespan.duration != c_TimeInfinite)
        {
            lifespan_duration_us_ =
                    duration<double, std::ratio<1, 1000000>>(m_att.qos.m_lifespan.duration.to_ns() * 1e-3);
            lifespan_timer_.update_interval_millisec(m_att.qos.m_lifespan.duration.to_ns() * 1e-6);
        }
        else
        {
            lifespan_timer_.cancel_timer();
        }
    }

    return updated;
}

void PublisherImpl::PublisherWriterListener::onWriterMatched(
        RTPSWriter* /*writer*/,
        MatchingInfo& info)
{
    if( mp_publisherImpl->mp_listener != nullptr )
    {
        mp_publisherImpl->mp_listener->onPublicationMatched(mp_publisherImpl->mp_userPublisher, info);
    }
}

void PublisherImpl::PublisherWriterListener::onWriterChangeReceivedByAll(
        RTPSWriter* /*writer*/,
        CacheChange_t* ch)
{
    if (mp_publisherImpl->m_att.qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
    {
        mp_publisherImpl->m_history.remove_change_g(ch);
    }
}

bool PublisherImpl::wait_for_all_acked(const eprosima::fastrtps::Time_t& max_wait)
{
    return mp_writer->wait_for_all_acked(max_wait);
}

void PublisherImpl::deadline_timer_reschedule()
{
    assert(m_att.qos.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(mp_writer->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!m_history.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(PUBLISHER, "Could not get the next deadline from the history");
        return;
    }
    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());

    deadline_timer_.cancel_timer();
    deadline_timer_.update_interval_millisec((double)interval_ms.count());
    deadline_timer_.restart_timer();
}

void PublisherImpl::deadline_missed()
{
    assert(m_att.qos.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(mp_writer->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    mp_listener->on_offered_deadline_missed(mp_userPublisher, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

    if (!m_history.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(PUBLISHER, "Could not set the next deadline in the history");
        return;
    }
    deadline_timer_reschedule();
}

void PublisherImpl::get_offered_deadline_missed_status(OfferedDeadlineMissedStatus &status)
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_writer->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
}

void PublisherImpl::lifespan_expired()
{
    std::unique_lock<std::recursive_timed_mutex> lock(mp_writer->getMutex());

    CacheChange_t* earliest_change;
    if (!m_history.get_earliest_change(&earliest_change))
    {
        return;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
    if (now - source_timestamp < lifespan_duration_us_)
    {
        auto interval = source_timestamp - now + lifespan_duration_us_;
        lifespan_timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
        lifespan_timer_.restart_timer();
        return;
    }

    // The earliest change has expired
    m_history.remove_change_pub(earliest_change);

    // Set the timer for the next change if there is one
    if (!m_history.get_earliest_change(&earliest_change))
    {
        return;
    }

    // Calculate when the next change is due to expire and restart
    source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    now = system_clock::now();
    auto interval = source_timestamp - now + lifespan_duration_us_;

    assert(interval.count() > 0);

    lifespan_timer_.update_interval_millisec((double)duration_cast<milliseconds>(interval).count());
    lifespan_timer_.restart_timer();
}
