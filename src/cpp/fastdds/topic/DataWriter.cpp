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
 * DataWriter.cpp
 *
 */

#include <fastdds/topic/DataWriter.hpp>
#include <fastrtps/topic/TopicDataType.h>
#include <fastrtps/topic/attributes/TopicAttributes.h>
#include <fastdds/publisher/PublisherListener.hpp>
#include <fastdds/publisher/Publisher.hpp>
#include "../publisher/PublisherImpl.hpp"

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastdds/domain/DomainParticipant.hpp>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/rtps/timedevent/TimedCallback.h>
#include <fastrtps/rtps/resources/ResourceEvent.h>

#include <functional>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {

DataWriter::DataWriter(
        PublisherImpl* p,
        TopicDataType* topic,
        const TopicAttributes& topic_att,
        const WriterAttributes& att,
        const WriterQos& qos,
        PublisherHistory&& history,
        DataWriterListener* listen )
    : publisher_(p)
    , writer_(nullptr)
    , type_(topic)
    , topic_att_(topic_att)
    , w_att_(att)
    , qos_(qos)
    , history_(std::move(history))
    , listener_(listen)
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , high_mark_for_frag_(0)
    , deadline_timer_(std::bind(&DataWriter::deadline_missed, this),
                      qos.m_deadline.period.to_ns() * 1e-6,
                      publisher_->get_participant()->get_resource_event().getIOService(),
                      publisher_->get_participant()->get_resource_event().getThread())
    , deadline_duration_us_(qos.m_deadline.period.to_ns() * 1e-3)
    , timer_owner_()
    , deadline_missed_status_()
    , lifespan_timer_(std::bind(&DataWriter::lifespan_expired, this),
                      qos.m_lifespan.duration.to_ns() * 1e-6,
                      publisher_->get_participant()->get_resource_event().getIOService(),
                      publisher_->get_participant()->get_resource_event().getThread())
    , lifespan_duration_us_(qos.m_lifespan.duration.to_ns() * 1e-3)
{
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(
                publisher_->rtps_participant(),
                w_att_,
                static_cast<WriterHistory*>(&history),
                static_cast<WriterListener*>(&writer_listener_));

    if(writer == nullptr)
    {
        logError(DATA_WRITER, "Problem creating associated Writer");
    }

    writer_ = writer;
}

DataWriter::~DataWriter()
{
    if(writer_ != nullptr)
    {
        logInfo(PUBLISHER, guid().entityId << " in topic: " << type_->getName());
    }

    RTPSDomain::removeRTPSWriter(writer_);
}

bool DataWriter::write(
        void* data)
{
    logInfo(DATA_WRITER, "Writing new data");
    return create_new_change(ALIVE, data);
}

bool DataWriter::write(
        void* data,
        rtps::WriteParams& params)
{
    logInfo(DATA_WRITER, "Writing new data with WriteParams");
    return create_new_change_with_params(ALIVE, data, params);
}

bool DataWriter::write(
            void* data,
            const rtps::InstanceHandle_t& handle)
{
    logInfo(DATA_WRITER, "Writing new data with Handle");
    WriteParams wparams;
    return create_new_change_with_params(ALIVE, data, wparams, handle);
}

bool DataWriter::dispose(
        void* data,
        const rtps::InstanceHandle_t& handle)
{
    logInfo(DATA_WRITER, "Disposing of data");
    WriteParams wparams;
    return create_new_change_with_params(NOT_ALIVE_DISPOSED, data, wparams, handle);
}

bool DataWriter::dispose(
        void* data)
{
    logInfo(DATA_WRITER, "Disposing of data");
    WriteParams wparams;
    return create_new_change_with_params(NOT_ALIVE_DISPOSED, data, wparams);
}

bool DataWriter::create_new_change(
        ChangeKind_t changeKind,
        void* data)
{
    WriteParams wparams;
    return create_new_change_with_params(changeKind, data, wparams);
}


bool DataWriter::check_new_change_preconditions(
        ChangeKind_t change_kind,
        void* data)
{
    // Preconditions
    if (data == nullptr)
    {
        logError(PUBLISHER, "Data pointer not valid");
        return false;
    }

    if(change_kind == NOT_ALIVE_UNREGISTERED || change_kind == NOT_ALIVE_DISPOSED ||
            change_kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        if(!type_->m_isGetKeyDefined)
        {
            logError(PUBLISHER,"Topic is NO_KEY, operation not permitted");
            return false;
        }
    }

    return true;
}

bool DataWriter::perform_create_new_change(
        ChangeKind_t change_kind,
        void* data,
        WriteParams& wparams,
        const InstanceHandle_t& handle)
{
    // Block lowlevel writer
    auto max_blocking_time = std::chrono::steady_clock::now() +
        std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.m_reliability.max_blocking_time));
    std::unique_lock<std::recursive_timed_mutex> lock(writer_->getMutex(), std::defer_lock);

    if(lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* ch = writer_->new_change(type_->getSerializedSizeProvider(data), change_kind, handle);
        if(ch != nullptr)
        {
            if(change_kind == ALIVE)
            {
                //If these two checks are correct, we asume the cachechange is valid and thwn we can write to it.
                if(!type_->serialize(data, &ch->serializedPayload))
                {
                    logWarning(RTPS_WRITER,"RTPSWriter:Serialization returns false";);
                    history_.release_Cache(ch);
                    return false;
                }
            }

            //TODO(Ricardo) This logic in a class. Then a user of rtps layer can use it.
            if(high_mark_for_frag_ == 0)
            {
                RTPSParticipant* part = publisher_->rtps_participant();
                uint32_t max_data_size = writer_->getMaxDataSize();
                uint32_t writer_throughput_controller_bytes =
                    writer_->calculateMaxDataSize(w_att_.throughputController.bytesPerPeriod);
                uint32_t participant_throughput_controller_bytes =
                    writer_->calculateMaxDataSize(
                        part-> getRTPSParticipantAttributes().throughputController.bytesPerPeriod);

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
                if( qos_.m_publishMode.kind != ASYNCHRONOUS_PUBLISH_MODE)
                {
                    logError(PUBLISHER, "Data cannot be sent. It's serialized size is " <<
                            ch->serializedPayload.length << "' which exceeds the maximum payload size of '" <<
                            final_high_mark_for_frag << "' and therefore ASYNCHRONOUS_PUBLISH_MODE must be used.");
                    history_.release_Cache(ch);
                    return false;
                }

                /// Fragment the data.
                // Set the fragment size to the cachechange.
                // Note: high_mark will always be a value that can be casted to uint16_t)
                ch->setFragmentSize(static_cast<uint16_t>(final_high_mark_for_frag));
            }

            if(!this->history_.add_pub_change(ch, wparams, lock, max_blocking_time))
            {
                history_.release_Cache(ch);
                return false;
            }

            if (qos_.m_deadline.period != c_TimeInfinite)
            {
                if (!history_.set_next_deadline(
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

            if (qos_.m_lifespan.duration != c_TimeInfinite)
            {
                lifespan_duration_us_ = std::chrono::duration<double, std::ratio<1, 1000000>>(qos_.m_lifespan.duration.to_ns() * 1e-3);
                lifespan_timer_.update_interval_millisec(qos_.m_lifespan.duration.to_ns() * 1e-6);
                lifespan_timer_.restart_timer();
            }

            return true;
        }
    }

    return false;
}

bool DataWriter::create_new_change_with_params(
        ChangeKind_t changeKind,
        void* data,
        WriteParams& wparams)
{
    if (!check_new_change_preconditions(changeKind, data))
    {
        return false;
    }

    InstanceHandle_t handle;
    if(type_->m_isGetKeyDefined)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif
        type_->getKey(data,&handle,is_key_protected);
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}

bool DataWriter::create_new_change_with_params(
        ChangeKind_t changeKind,
        void* data,
        WriteParams& wparams,
        const rtps::InstanceHandle_t& handle)
{
    if (!check_new_change_preconditions(changeKind, data))
    {
        return false;
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}


bool DataWriter::remove_min_seq_change()
{
    return history_.removeMinChange();
}

bool DataWriter::remove_all_change(size_t* removed)
{
    return history_.removeAllChange(removed);
}

const GUID_t& DataWriter::guid()
{
    return writer_->getGuid();
}

InstanceHandle_t DataWriter::get_instance_handle() const
{
    InstanceHandle_t handle;
    handle = writer_->getGuid();
    return handle;
}

bool DataWriter::set_attributes(const WriterAttributes& att)
{
    bool updated = true;
    bool missing = false;

    if(qos_.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
    {
        if(att.endpoint.unicastLocatorList.size() != w_att_.endpoint.unicastLocatorList.size() ||
                att.endpoint.multicastLocatorList.size() != w_att_.endpoint.multicastLocatorList.size())
        {
            logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
            updated &= false;
        }
        else
        {
            for(LocatorListConstIterator lit1 = w_att_.endpoint.unicastLocatorList.begin();
                    lit1!=this->w_att_.endpoint.unicastLocatorList.end();++lit1)
            {
                missing = true;
                for(LocatorListConstIterator lit2 = att.endpoint.unicastLocatorList.begin();
                        lit2!= att.endpoint.unicastLocatorList.end();++lit2)
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
            for(LocatorListConstIterator lit1 = this->w_att_.endpoint.multicastLocatorList.begin();
                    lit1!=this->w_att_.endpoint.multicastLocatorList.end();++lit1)
            {
                missing = true;
                for(LocatorListConstIterator lit2 = att.endpoint.multicastLocatorList.begin();
                        lit2!= att.endpoint.multicastLocatorList.end();++lit2)
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
        if(qos_.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulWriter* sfw = static_cast<StatefulWriter*>(writer_);
            sfw->updateTimes(att.times);
        }

        this->w_att_ = att;
    }

    return updated;
}

const WriterAttributes& DataWriter::get_attributes() const
{
    return w_att_;
}

bool DataWriter::set_qos(const WriterQos& qos)
{
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if(!qos_.canQosBeUpdated(qos))
    {
        return false;
    }

    qos_.setQos(qos,false);
    //Notify the participant that a Writer has changed its QOS
    publisher_->rtps_participant()->updateWriter(writer_, topic_att_, qos_);
    //publisher_->update_writer(this, topic_att_, qos_);

    // Deadline
    if (qos_.m_deadline.period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000>>(qos_.m_deadline.period.to_ns() * 1e-3);
        deadline_timer_.update_interval_millisec(qos_.m_deadline.period.to_ns() * 1e-6);
    }
    else
    {
        deadline_timer_.cancel_timer();
    }

    // Lifespan
    if (qos_.m_lifespan.duration != c_TimeInfinite)
    {
        lifespan_duration_us_ =
                duration<double, std::ratio<1, 1000000>>(qos_.m_lifespan.duration.to_ns() * 1e-3);
        lifespan_timer_.update_interval_millisec(qos_.m_lifespan.duration.to_ns() * 1e-6);
    }
    else
    {
        lifespan_timer_.cancel_timer();
    }

    return true;
}

const WriterQos& DataWriter::get_qos() const
{
    return qos_;
}

bool DataWriter::set_listener(DataWriterListener* listener)
{
    if (listener_ == listener)
    {
        return false;
    }

    listener_ = listener;
    return true;
}

const DataWriterListener* DataWriter::get_listener() const
{
    return listener_;
}

bool DataWriter::set_topic(const TopicAttributes& att)
{
    //TOPIC ATTRIBUTES
    if(topic_att_ != att)
    {
        logWarning(DATA_WRITER, "Topic Attributes cannot be updated");
        return false;
    }
    //publisher_->update_writer(this, topic_att_, qos_);
    publisher_->rtps_participant()->updateWriter(writer_, topic_att_, qos_);
    return true;
}

const TopicAttributes& DataWriter::get_topic() const
{
    return topic_att_;
}

const Publisher* DataWriter::get_publisher() const
{
    return publisher_->get_publisher();
}

void DataWriter::InnerDataWriterListener::on_writer_matched(
        RTPSWriter* /*writer*/,
        MatchingInfo& info)
{
    if( data_writer_->listener_ != nullptr )
    {
        data_writer_->listener_->on_publication_matched(
            data_writer_, info);
    }
}

void DataWriter::InnerDataWriterListener::on_writer_change_received_by_all(
        RTPSWriter* /*writer*/,
        CacheChange_t* ch)
{
    if (data_writer_->qos_.m_durability.kind == VOLATILE_DURABILITY_QOS)
    {
        data_writer_->history_.remove_change_g(ch);
    }
}

bool DataWriter::wait_for_acknowledgments(
        const Duration_t &max_wait)
{
    return writer_->wait_for_all_acked(max_wait);
}

void DataWriter::deadline_timer_reschedule()
{
    assert(qos_.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(writer_->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!history_.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(PUBLISHER, "Could not get the next deadline from the history");
        return;
    }
    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());

    deadline_timer_.cancel_timer();
    deadline_timer_.update_interval_millisec(static_cast<double>(interval_ms.count()));
    deadline_timer_.restart_timer();
}

void DataWriter::deadline_missed()
{
    assert(qos_.m_deadline.period != c_TimeInfinite);

    std::unique_lock<std::recursive_timed_mutex> lock(writer_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    listener_->on_offered_deadline_missed(this, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

    if (!history_.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(PUBLISHER, "Could not set the next deadline in the history");
        return;
    }
    deadline_timer_reschedule();
}

void DataWriter::get_offered_deadline_missed_status(OfferedDeadlineMissedStatus &status)
{
    std::unique_lock<std::recursive_timed_mutex> lock(writer_->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
}

void DataWriter::lifespan_expired()
{
    std::unique_lock<std::recursive_timed_mutex> lock(writer_->getMutex());

    CacheChange_t* earliest_change;
    if (!history_.get_earliest_change(&earliest_change))
    {
        return;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
    if (now - source_timestamp < lifespan_duration_us_)
    {
        auto interval = source_timestamp - now + lifespan_duration_us_;
        lifespan_timer_.update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
        lifespan_timer_.restart_timer();
        return;
    }

    // The earliest change has expired
    history_.remove_change_pub(earliest_change);

    // Set the timer for the next change if there is one
    if (!history_.get_earliest_change(&earliest_change))
    {
        return;
    }

    // Calculate when the next change is due to expire and restart
    source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    now = system_clock::now();
    auto interval = source_timestamp - now + lifespan_duration_us_;

    assert(interval.count() > 0);

    lifespan_timer_.update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
    lifespan_timer_.restart_timer();
}

bool DataWriter::assert_liveliness()
{
    writer_->setLivelinessAsserted(true);
    return true;
}

} // namespace fastdds
} // namespace eprosima
