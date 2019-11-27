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
 * DataWriterImpl.cpp
 *
 */

#include <fastdds/topic/DataWriterImpl.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/DataWriter.hpp>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastdds/publisher/PublisherImpl.hpp>

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>

#include <fastrtps/log/Log.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastdds/rtps/resources/ResourceEvent.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <functional>
#include <iostream>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace dds {

DataWriterImpl::DataWriterImpl(
        PublisherImpl* p,
        TypeSupport type,
        const TopicAttributes& topic_att,
        const WriterAttributes& att,
        const DataWriterQos& qos,
        const MemoryManagementPolicy_t memory_policy,
        DataWriterListener* listen )
    : publisher_(p)
    , writer_(nullptr)
    , type_(type)
    , topic_att_(topic_att)
    , w_att_(att)
    , qos_(&qos == &DDS_DATAWRITER_QOS_DEFAULT ? publisher_->get_default_datawriter_qos() : qos)
    , history_(topic_att_, type_.get()->m_typeSize
#if HAVE_SECURITY
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            + 20 /*SecureDataHeader*/ + 4 + ((2* 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/
#endif
            , memory_policy)
    //, history_(std::move(history))
    , listener_(listen)
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , high_mark_for_frag_(0)
    , deadline_duration_us_(qos_.deadline.period.to_ns() * 1e-3)
    , timer_owner_()
    , deadline_missed_status_()
    , lifespan_duration_us_(qos_.lifespan.duration.to_ns() * 1e-3)
    , user_datawriter_(nullptr)
{
    deadline_timer_ = new TimedEvent(publisher_->get_participant()->get_resource_event(),
                                     [&](TimedEvent::EventCode code) -> bool
                                     {
                                         if (TimedEvent::EVENT_SUCCESS == code)
                                         {
                                             return deadline_missed();
                                         }

                                         return false;
                                     },
                                     qos_.deadline.period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(publisher_->get_participant()->get_resource_event(),
                    [&](TimedEvent::EventCode code) -> bool
                {
                    if (TimedEvent::EVENT_SUCCESS == code)
                    {
                        return lifespan_expired();
                    }

                     return false;
                 },
                     qos_.lifespan.duration.to_ns() * 1e-6);

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(
        publisher_->rtps_participant(),
        w_att_,
        static_cast<WriterHistory*>(&history_),
        static_cast<WriterListener*>(&writer_listener_));

    if (writer == nullptr)
    {
        logError(DATA_WRITER, "Problem creating associated Writer");
    }

    writer_ = writer;
}

void DataWriterImpl::disable()
{
    set_listener(nullptr);
    if (writer_ != nullptr)
    {
        writer_->set_listener(nullptr);
    }
}

DataWriterImpl::~DataWriterImpl()
{
    delete lifespan_timer_;
    delete deadline_timer_;

    if (writer_ != nullptr)
    {
        logInfo(PUBLISHER, guid().entityId << " in topic: " << type_.get_type_name());
    }

    RTPSDomain::removeRTPSWriter(writer_);
    delete user_datawriter_;
}

bool DataWriterImpl::write(
        void* data)
{
    logInfo(DATA_WRITER, "Writing new data");
    return create_new_change(ALIVE, data);
}

bool DataWriterImpl::write(
        void* data,
        fastrtps::rtps::WriteParams& params)
{
    logInfo(DATA_WRITER, "Writing new data with WriteParams");
    return create_new_change_with_params(ALIVE, data, params);
}

ReturnCode_t DataWriterImpl::write(
        void* data,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    //TODO Review when HANDLE_NIL is implemented as this just checks if the handle is 0,
    // but it need to check if there is an existing entity with that handle
    if (!handle.isDefined())
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
    logInfo(DATA_WRITER, "Writing new data with Handle");
    WriteParams wparams;
    if (create_new_change_with_params(ALIVE, data, wparams, handle))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

ReturnCode_t DataWriterImpl::dispose(
        void* data,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    if (!handle.isDefined())
    {
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
    logInfo(DATA_WRITER, "Disposing of data");
    WriteParams wparams;
    if (create_new_change_with_params(NOT_ALIVE_DISPOSED, data, wparams, handle))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

bool DataWriterImpl::dispose(
        void* data)
{
    logInfo(DATA_WRITER, "Disposing of data");
    WriteParams wparams;
    return create_new_change_with_params(NOT_ALIVE_DISPOSED, data, wparams);
}

bool DataWriterImpl::create_new_change(
        ChangeKind_t changeKind,
        void* data)
{
    WriteParams wparams;
    return create_new_change_with_params(changeKind, data, wparams);
}


bool DataWriterImpl::check_new_change_preconditions(
        ChangeKind_t change_kind,
        void* data)
{
    // Preconditions
    if (data == nullptr)
    {
        logError(PUBLISHER, "Data pointer not valid");
        return false;
    }

    if (change_kind == NOT_ALIVE_UNREGISTERED
            || change_kind == NOT_ALIVE_DISPOSED
            || change_kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        if (!type_.get()->m_isGetKeyDefined)
        {
            logError(PUBLISHER,"Topic is NO_KEY, operation not permitted");
            return false;
        }
    }

    return true;
}

bool DataWriterImpl::perform_create_new_change(
        ChangeKind_t change_kind,
        void* data,
        WriteParams& wparams,
        const InstanceHandle_t& handle)
{
    // Block lowlevel writer
    auto max_blocking_time = std::chrono::steady_clock::now() +
        std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability.max_blocking_time));

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (lock.try_lock_until(max_blocking_time))
    {
        CacheChange_t* ch = writer_->new_change(type_.get()->getSerializedSizeProvider(data), change_kind, handle);
        if (ch != nullptr)
        {
            if (change_kind == ALIVE)
            {
                //If these two checks are correct, we asume the cachechange is valid and thwn we can write to it.
                if (!type_.serialize(data, &ch->serializedPayload))
                {
                    logWarning(RTPS_WRITER,"RTPSWriter:Serialization returns false"; );
                    history_.release_Cache(ch);
                    return false;
                }
            }

            //TODO(Ricardo) This logic in a class. Then a user of rtps layer can use it.
            if (high_mark_for_frag_ == 0)
            {
                RTPSParticipant* part = publisher_->rtps_participant();
                uint32_t max_data_size = writer_->getMaxDataSize();
                uint32_t writer_throughput_controller_bytes =
                        writer_->calculateMaxDataSize(w_att_.throughputController.bytesPerPeriod);
                uint32_t participant_throughput_controller_bytes =
                        writer_->calculateMaxDataSize(
                    part->getRTPSParticipantAttributes().throughputController.bytesPerPeriod);

                high_mark_for_frag_ =
                        max_data_size > writer_throughput_controller_bytes ?
                        writer_throughput_controller_bytes :
                        (max_data_size > participant_throughput_controller_bytes ?
                        participant_throughput_controller_bytes :
                        max_data_size);
            }

            uint32_t final_high_mark_for_frag = high_mark_for_frag_;

            // If needed inlineqos for related_sample_identity, then remove the inlinqos size from final fragment size.
            if (wparams.related_sample_identity() != SampleIdentity::unknown())
            {
                final_high_mark_for_frag -= 32;
            }

            // If it is big data, fragment it.
            if (ch->serializedPayload.length > final_high_mark_for_frag)
            {
                // Check ASYNCHRONOUS_PUBLISH_MODE is being used, but it is an error case.
                if (qos_.publish_mode.kind != ASYNCHRONOUS_PUBLISH_MODE)
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

            if (!this->history_.add_pub_change(ch, wparams, lock, max_blocking_time))
            {
                history_.release_Cache(ch);
                return false;
            }

            if (qos_.deadline.period != c_TimeInfinite)
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

            if (qos_.lifespan.duration != c_TimeInfinite)
            {
                lifespan_duration_us_ = std::chrono::duration<double, std::ratio<1, 1000000>>(qos_.lifespan.duration.to_ns() * 1e-3);
                lifespan_timer_->update_interval_millisec(qos_.lifespan.duration.to_ns() * 1e-6);
            }
            else
            {
                lifespan_timer_->cancel_timer();
            }

            return true;
        }
    }

    return false;
}

bool DataWriterImpl::create_new_change_with_params(
        ChangeKind_t changeKind,
        void* data,
        WriteParams& wparams)
{
    if (!check_new_change_preconditions(changeKind, data))
    {
        return false;
    }

    InstanceHandle_t handle;
    if (type_.get()->m_isGetKeyDefined)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif
        type_.get()->getKey(data,&handle,is_key_protected);
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}

bool DataWriterImpl::create_new_change_with_params(
        ChangeKind_t changeKind,
        void* data,
        WriteParams& wparams,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    if (!check_new_change_preconditions(changeKind, data))
    {
        return false;
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}


bool DataWriterImpl::remove_min_seq_change()
{
    return history_.removeMinChange();
}

bool DataWriterImpl::remove_all_change(
        size_t* removed)
{
    return history_.removeAllChange(removed);
}

const GUID_t& DataWriterImpl::guid()
{
    return writer_->getGuid();
}

InstanceHandle_t DataWriterImpl::get_instance_handle() const
{
    InstanceHandle_t handle;
    handle = writer_->getGuid();
    return handle;
}

bool DataWriterImpl::set_attributes(
        const WriterAttributes& att)
{
    bool updated = true;
    bool missing = false;

    if (qos_.reliability.kind == RELIABLE_RELIABILITY_QOS)
    {
        if (att.endpoint.unicastLocatorList.size() != w_att_.endpoint.unicastLocatorList.size() ||
                att.endpoint.multicastLocatorList.size() != w_att_.endpoint.multicastLocatorList.size())
        {
            logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
            updated &= false;
        }
        else
        {
            for (LocatorListConstIterator lit1 = w_att_.endpoint.unicastLocatorList.begin();
                    lit1!=this->w_att_.endpoint.unicastLocatorList.end(); ++lit1)
            {
                missing = true;
                for (LocatorListConstIterator lit2 = att.endpoint.unicastLocatorList.begin();
                        lit2!= att.endpoint.unicastLocatorList.end(); ++lit2)
                {
                    if (*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if (missing)
                {
                    logWarning(PUBLISHER,"Locator: "<< *lit1 << " not present in new list");
                    logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
                }
            }
            for (LocatorListConstIterator lit1 = this->w_att_.endpoint.multicastLocatorList.begin();
                    lit1!=this->w_att_.endpoint.multicastLocatorList.end(); ++lit1)
            {
                missing = true;
                for (LocatorListConstIterator lit2 = att.endpoint.multicastLocatorList.begin();
                        lit2!= att.endpoint.multicastLocatorList.end(); ++lit2)
                {
                    if (*lit1 == *lit2)
                    {
                        missing = false;
                        break;
                    }
                }
                if (missing)
                {
                    logWarning(PUBLISHER,"Locator: "<< *lit1<< " not present in new list");
                    logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
                }
            }
        }
    }

    if (updated)
    {
        if (qos_.reliability.kind == RELIABLE_RELIABILITY_QOS)
        {
            //UPDATE TIMES:
            StatefulWriter* sfw = static_cast<StatefulWriter*>(writer_);
            sfw->updateTimes(att.times);
        }

        this->w_att_ = att;
    }

    return updated;
}

const WriterAttributes& DataWriterImpl::get_attributes() const
{
    return w_att_;
}

ReturnCode_t DataWriterImpl::set_qos(
        const DataWriterQos& qos)
{
    //QOS:
    //CHECK IF THE QOS CAN BE SET
    if (!qos.checkQos())
    {
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    else if (!qos_.canQosBeUpdated(qos))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }

    qos_.setQos(qos,false);
    //Notify the participant that a Writer has changed its QOS
    fastrtps::WriterQos wqos_ = qos_.changeToWriterQos();
    publisher_->rtps_participant()->updateWriter(writer_, topic_att_, wqos_);
    //publisher_->update_writer(this, topic_att_, qos_);

    // Deadline
    if (qos_.deadline.period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000>>(qos_.deadline.period.to_ns() * 1e-3);
        deadline_timer_->update_interval_millisec(qos_.deadline.period.to_ns() * 1e-6);
    }
    else
    {
        deadline_timer_->cancel_timer();
    }

    // Lifespan
    if (qos_.lifespan.duration != c_TimeInfinite)
    {
        lifespan_duration_us_ =
                duration<double, std::ratio<1, 1000000>>(qos_.lifespan.duration.to_ns() * 1e-3);
        lifespan_timer_->update_interval_millisec(qos_.lifespan.duration.to_ns() * 1e-6);
    }
    else
    {
        lifespan_timer_->cancel_timer();
    }

    return ReturnCode_t::RETCODE_OK;
}

const DataWriterQos& DataWriterImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t DataWriterImpl::set_listener(
        DataWriterListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

const DataWriterListener* DataWriterImpl::get_listener() const
{
    return listener_;
}

bool DataWriterImpl::set_topic(
        const TopicAttributes& att)
{
    //TOPIC ATTRIBUTES
    if (topic_att_ != att)
    {
        logWarning(DATA_WRITER, "Topic Attributes cannot be updated");
        return false;
    }
    //publisher_->update_writer(this, topic_att_, qos_);
    //publisher_->rtps_participant()->updateWriter(writer_, topic_att_, qos_);
    return true;
}

const TopicAttributes& DataWriterImpl::get_topic() const
{
    return topic_att_;
}

const Publisher* DataWriterImpl::get_publisher() const
{
    return publisher_->get_publisher();
}

void DataWriterImpl::InnerDataWriterListener::onWriterMatched(
        RTPSWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (data_writer_->listener_ != nullptr )
    {
        data_writer_->listener_->on_publication_matched(
            data_writer_->user_datawriter_, info);
    }

    data_writer_->publisher_->publisher_listener_.on_publication_matched(data_writer_->user_datawriter_, info);
}

void DataWriterImpl::InnerDataWriterListener::onWriterChangeReceivedByAll(
        RTPSWriter* /*writer*/,
        CacheChange_t* ch)
{
    if (data_writer_->qos_.durability.kind == VOLATILE_DURABILITY_QOS)
    {
        data_writer_->history_.remove_change_g(ch);
    }
}

void DataWriterImpl::InnerDataWriterListener::on_liveliness_lost(
        fastrtps::rtps::RTPSWriter* /*writer*/,
        const fastrtps::LivelinessLostStatus& status)
{
    if (data_writer_->listener_ != nullptr)
    {
        data_writer_->listener_->on_liveliness_lost(data_writer_->user_datawriter_, status);
    }

    data_writer_->publisher_->publisher_listener_.on_liveliness_lost(data_writer_->user_datawriter_, status);
}

ReturnCode_t DataWriterImpl::wait_for_acknowledgments(
        const Duration_t& max_wait)
{
    if (writer_->wait_for_all_acked(max_wait))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

bool DataWriterImpl::deadline_timer_reschedule()
{
    assert(qos_.deadline.period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!history_.get_next_deadline(timer_owner_, next_deadline_us))
    {
        logError(PUBLISHER, "Could not get the next deadline from the history");
        return false;
    }

    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());
    deadline_timer_->update_interval_millisec(static_cast<double>(interval_ms.count()));
    return true;
}

bool DataWriterImpl::deadline_missed()
{
    assert(qos_.deadline.period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    listener_->on_offered_deadline_missed(user_datawriter_, deadline_missed_status_);
    publisher_->publisher_listener_.on_offered_deadline_missed(user_datawriter_, deadline_missed_status_);
    deadline_missed_status_.total_count_change = 0;

    if (!history_.set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<system_clock::duration>(deadline_duration_us_)))
    {
        logError(PUBLISHER, "Could not set the next deadline in the history");
        return false;
    }
    return deadline_timer_reschedule();
}

ReturnCode_t DataWriterImpl::get_offered_deadline_missed_status(
        OfferedDeadlineMissedStatus& status)
{
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriterImpl::get_offered_incompatible_qos_status(
        OfferedIncompatibleQosStatus& status)
{
    status = writer_->offered_incompatible_qos_status_;
    writer_->offered_incompatible_qos_status_.total_count_change = 0;
    return ReturnCode_t::RETCODE_OK;
}

bool DataWriterImpl::lifespan_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    CacheChange_t* earliest_change;
    if (!history_.get_earliest_change(&earliest_change))
    {
        return false;
    }

    auto source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    auto now = system_clock::now();

    // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
    if (now - source_timestamp < lifespan_duration_us_)
    {
        auto interval = source_timestamp - now + lifespan_duration_us_;
        lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
        return true;
    }

    // The earliest change has expired
    history_.remove_change_pub(earliest_change);

    // Set the timer for the next change if there is one
    if (!history_.get_earliest_change(&earliest_change))
    {
        return false;
    }

    // Calculate when the next change is due to expire and restart
    source_timestamp = system_clock::time_point() + nanoseconds(earliest_change->sourceTimestamp.to_ns());
    now = system_clock::now();
    auto interval = source_timestamp - now + lifespan_duration_us_;

    assert(interval.count() > 0);

    lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
    return true;
}

ReturnCode_t DataWriterImpl::get_liveliness_lost_status(
        LivelinessLostStatus& status)
{
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    status.total_count = writer_->liveliness_lost_status_.total_count;
    status.total_count_change = writer_->liveliness_lost_status_.total_count_change;

    writer_->liveliness_lost_status_.total_count_change = 0u;

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriterImpl::assert_liveliness()
{
    if (!publisher_->rtps_participant()->wlp()->assert_liveliness(
                writer_->getGuid(),
                writer_->get_liveliness_kind(),
                writer_->get_liveliness_lease_duration()))
    {
        logError(DATAWRITER, "Could not assert liveliness of writer " << writer_->getGuid());
        return ReturnCode_t::RETCODE_ERROR;
    }

    if (qos_.liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        // As described in the RTPS specification, if liveliness kind is manual a heartbeat must be sent
        // This only applies to stateful writers, as stateless writers do not send heartbeats

        StatefulWriter* stateful_writer = dynamic_cast<StatefulWriter*>(writer_);

        if (stateful_writer != nullptr)
        {
            stateful_writer->send_periodic_heartbeat(true, true);
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriterImpl::get_publication_matched_status(
        PublicationMatchedStatus& status)
{
    status = writer_->publication_matched_status_;
    return ReturnCode_t::RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
