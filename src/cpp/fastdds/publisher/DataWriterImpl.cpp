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

#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>

#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>

#include <fastdds/dds/log/Log.hpp>
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
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listen)
    : publisher_(p)
    , writer_(nullptr)
    , type_(type)
    , topic_(topic)
    , qos_(&qos == &DATAWRITER_QOS_DEFAULT ? publisher_->get_default_datawriter_qos() : qos)
    , history_(get_topic_attributes(qos_, *topic_, type_), type_->m_typeSize
#if HAVE_SECURITY
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            + 20 /*SecureDataHeader*/ + 4 + ((2 * 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/
#endif
            , qos.endpoint_data().history_memory_policy)
    //, history_(std::move(history))
    , listener_(listen)
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , high_mark_for_frag_(0)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , timer_owner_()
    , deadline_missed_status_()
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
    , user_datawriter_(nullptr)
{
    deadline_timer_ = new TimedEvent(publisher_->get_participant()->get_resource_event(),
                    [&]() -> bool
                {
                    return deadline_missed();
                },
                    qos_.deadline().period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(publisher_->get_participant()->get_resource_event(),
                    [&]() -> bool
                {
                    return lifespan_expired();
                },
                    qos_.lifespan().duration.to_ns() * 1e-6);

    WriterAttributes w_att;
    w_att.throughputController = qos.throughput_controller();
    w_att.endpoint.durabilityKind = qos.durability().durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    w_att.endpoint.multicastLocatorList = qos.endpoint_data().multicast_locator_list;
    w_att.endpoint.reliabilityKind = qos.reliability().kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = type->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    w_att.endpoint.unicastLocatorList = qos.endpoint_data().unicast_locator_list;
    w_att.endpoint.remoteLocatorList = qos.endpoint_data().remote_locator_list;
    w_att.mode = qos.publish_mode().kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
    w_att.endpoint.properties = qos.properties();

    if (qos.endpoint_data().entity_id > 0)
    {
        w_att.endpoint.setEntityID(static_cast<uint8_t>(qos.endpoint_data().entity_id));
    }

    if (qos.endpoint_data().user_defined_id > 0)
    {
        w_att.endpoint.setUserDefinedID(static_cast<uint8_t>(qos.endpoint_data().user_defined_id));
    }

    w_att.times = qos.reliable_writer_data().times;
    w_att.liveliness_kind = qos.liveliness().kind;
    w_att.liveliness_lease_duration = qos.liveliness().lease_duration;
    w_att.matched_readers_allocation = qos.writer_resources().matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic->get_name().c_str());
    w_att.endpoint.properties.properties().push_back(std::move(property));

    if (publisher_->get_qos().partition().names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for (auto partition : publisher_->get_qos().partition().names())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        w_att.endpoint.properties.properties().push_back(std::move(property));
    }

    if (qos.reliable_writer_data().disable_positive_acks.enabled &&
            qos.reliable_writer_data().disable_positive_acks.duration != c_TimeInfinite)
    {
        w_att.disable_positive_acks = true;
        w_att.keep_duration = qos.reliable_writer_data().disable_positive_acks.duration;
    }

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(
        publisher_->rtps_participant(),
        w_att,
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
        logInfo(PUBLISHER, guid().entityId << " in topic: " << type_->getName());
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
        if (!type_->m_isGetKeyDefined)
        {
            logError(PUBLISHER, "Topic is NO_KEY, operation not permitted");
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
    auto max_blocking_time = steady_clock::now() +
            microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (lock.try_lock_until(max_blocking_time))
#else
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());
#endif
    {
        CacheChange_t* ch = writer_->new_change(type_->getSerializedSizeProvider(data), change_kind, handle);
        if (ch != nullptr)
        {
            if (change_kind == ALIVE)
            {
                //If these two checks are correct, we asume the cachechange is valid and thwn we can write to it.
                if (!type_->serialize(data, &ch->serializedPayload))
                {
                    logWarning(RTPS_WRITER, "RTPSWriter:Serialization returns false"; );
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
                        writer_->calculateMaxDataSize(qos_.throughput_controller().bytesPerPeriod);
                uint32_t participant_throughput_controller_bytes =
                        writer_->calculateMaxDataSize(
                    part->getRTPSParticipantAttributes().throughputController.bytesPerPeriod);

                high_mark_for_frag_ =
                        max_data_size > writer_throughput_controller_bytes ?
                        writer_throughput_controller_bytes :
                        (max_data_size > participant_throughput_controller_bytes ?
                        participant_throughput_controller_bytes :
                        max_data_size);
                high_mark_for_frag_ &= ~3;
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
                // Fragment the data.
                // Set the fragment size to the cachechange.
                ch->setFragmentSize(static_cast<uint16_t>(
                            (std::min)(final_high_mark_for_frag, RTPSMessageGroup::get_max_fragment_payload_size())));
            }

            if (!this->history_.add_pub_change(ch, wparams, lock, max_blocking_time))
            {
                history_.release_Cache(ch);
                return false;
            }

            if (qos_.deadline().period != c_TimeInfinite)
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
                        if (deadline_timer_reschedule())
                        {
                            deadline_timer_->cancel_timer();
                            deadline_timer_->restart_timer();
                        }
                    }
                }
            }

            if (qos_.lifespan().duration != c_TimeInfinite)
            {
                lifespan_duration_us_ = duration<double, std::ratio<1, 1000000> >(
                    qos_.lifespan().duration.to_ns() * 1e-3);
                lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
                lifespan_timer_->restart_timer();
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
    if (type_->m_isGetKeyDefined)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif
        type_->getKey(data, &handle, is_key_protected);
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

ReturnCode_t DataWriterImpl::set_qos(
        const DataWriterQos& qos)
{
    if (&qos == &DATAWRITER_QOS_DEFAULT)
    {
        const DataWriterQos& default_qos = publisher_->get_default_datawriter_qos();
        if (!can_qos_be_updated(qos_, default_qos))
        {
            return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
        }
        set_qos(qos_, default_qos, false);
    }

    bool update_user_data = false;
    if (publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data == 0 ||
            publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data >
            qos.user_data().getValue().size())
    {
        update_user_data = true;
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
    set_qos(qos_, qos, false, update_user_data);

    //Notify the participant that a Writer has changed its QOS
    fastrtps::TopicAttributes topic_att = get_topic_attributes(qos_, *topic_, type_);
    WriterQos wqos = qos.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
    publisher_->rtps_participant()->updateWriter(writer_, topic_att, wqos);

    // Deadline
    if (qos_.deadline().period != c_TimeInfinite)
    {
        deadline_duration_us_ =
                duration<double, std::ratio<1, 1000000> >(qos_.deadline().period.to_ns() * 1e-3);
        deadline_timer_->update_interval_millisec(qos_.deadline().period.to_ns() * 1e-6);
    }
    else
    {
        deadline_timer_->cancel_timer();
    }

    // Lifespan
    if (qos_.lifespan().duration != c_TimeInfinite)
    {
        lifespan_duration_us_ =
                duration<double, std::ratio<1, 1000000> >(qos_.lifespan().duration.to_ns() * 1e-3);
        lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
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

Topic* DataWriterImpl::get_topic() const
{
    return topic_;
}

const Publisher* DataWriterImpl::get_publisher() const
{
    return publisher_->get_publisher();
}

void DataWriterImpl::InnerDataWriterListener::onWriterMatched(
        RTPSWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (data_writer_->listener_ != nullptr)
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
    if (data_writer_->qos_.durability().kind == VOLATILE_DURABILITY_QOS)
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
    assert(qos_.deadline().period != c_TimeInfinite);

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
    assert(qos_.deadline().period != c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    if (listener_ != nullptr)
    {
        listener_->on_offered_deadline_missed(user_datawriter_, deadline_missed_status_);
    }
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

bool DataWriterImpl::lifespan_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    CacheChange_t* earliest_change;
    while (history_.get_earliest_change(&earliest_change))
    {
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

        if (interval.count() > 0)
        {
            lifespan_timer_->update_interval_millisec(static_cast<double>(duration_cast<milliseconds>(interval).count()));
            return true;
        }
    }

    return false;
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

    if (qos_.liveliness().kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
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

fastrtps::TopicAttributes DataWriterImpl::get_topic_attributes(
        const DataWriterQos& qos,
        const Topic& topic,
        const TypeSupport& type)
{
    fastrtps::TopicAttributes topic_att;
    topic_att.historyQos = qos.history();
    topic_att.resourceLimitsQos = qos.resource_limits();
    topic_att.topicName = topic.get_name();
    topic_att.topicDataType = topic.get_type_name();
    topic_att.topicKind = type->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    topic_att.auto_fill_type_information = type->auto_fill_type_information();
    topic_att.auto_fill_type_object = type->auto_fill_type_object();
    if (type->type_identifier())
    {
        topic_att.type_id = *type->type_identifier();
    }
    if (type->type_object())
    {
        topic_att.type = *type->type_object();
    }
    if (type->type_information())
    {
        topic_att.type_information = *type->type_information();
    }
    return topic_att;
}

void DataWriterImpl::set_qos(
        DataWriterQos& to,
        const DataWriterQos& from,
        bool is_default,
        bool update_user_data)
{
    if (is_default && !(to.durability() == from.durability()))
    {
        to.durability() = from.durability();
        to.durability().hasChanged = true;
    }
    if (is_default && !(to.durability_service() == from.durability_service()))
    {
        to.durability_service() = from.durability_service();
        to.durability_service().hasChanged = true;
    }
    if (!(to.deadline() == from.deadline()))
    {
        to.deadline() = from.deadline();
        to.deadline().hasChanged = true;
    }
    if (!(to.latency_budget() == from.latency_budget()))
    {
        to.latency_budget() = from.latency_budget();
        to.latency_budget().hasChanged = true;
    }
    if (is_default && !(to.liveliness() == from.liveliness()))
    {
        to.liveliness() = from.liveliness();
        to.liveliness().hasChanged = true;
    }
    if (is_default && !(to.reliability() == from.reliability()))
    {
        to.reliability() = from.reliability();
        to.reliability().hasChanged = true;
    }
    if (is_default && !(to.destination_order() == from.destination_order()))
    {
        to.destination_order() = from.destination_order();
        to.destination_order().hasChanged = true;
    }
    if (is_default && !(to.history() == from.history()))
    {
        to.history() = from.history();
        to.history().hasChanged = true;
    }
    if (is_default && !(to.resource_limits() == from.resource_limits()))
    {
        to.resource_limits() = from.resource_limits();
        to.resource_limits().hasChanged = true;
    }
    if (!(to.transport_priority() == from.transport_priority()))
    {
        to.transport_priority() = from.transport_priority();
        to.transport_priority().hasChanged = true;
    }
    if (!(to.lifespan() == from.lifespan()))
    {
        to.lifespan() = from.lifespan();
        to.lifespan().hasChanged = true;
    }
    if (update_user_data && !(to.user_data() == from.user_data()))
    {
        to.user_data() = from.user_data();
        to.user_data().hasChanged = true;
    }
    if (is_default && !(to.ownership() == from.ownership()))
    {
        to.ownership() = from.ownership();
        to.ownership().hasChanged = true;
    }
    if (!(to.ownership_strength() == from.ownership_strength()))
    {
        to.ownership_strength() = from.ownership_strength();
        to.ownership_strength().hasChanged = true;
    }
    if (!(to.writer_data_lifecycle() == from.writer_data_lifecycle()))
    {
        to.writer_data_lifecycle() = from.writer_data_lifecycle();
    }
    if (is_default && !(to.publish_mode() == from.publish_mode()))
    {
        to.publish_mode() = from.publish_mode();
    }
    if (!(to.representation() == from.representation()))
    {
        to.representation() = from.representation();
        to.representation().hasChanged = true;
    }
    if (is_default && !(to.properties() == from.properties()))
    {
        to.properties() = from.properties();
    }
    if (is_default && !(to.reliable_writer_data() == from.reliable_writer_data()))
    {
        to.reliable_writer_data() = from.reliable_writer_data();
    }
    if (is_default && !(to.endpoint_data() == from.endpoint_data()))
    {
        to.endpoint_data() = from.endpoint_data();
    }
    if (is_default && !(to.writer_resources() == from.writer_resources()))
    {
        to.writer_resources() = from.writer_resources();
    }
    if (is_default && !(to.throughput_controller() == from.throughput_controller()))
    {
        to.throughput_controller() = from.throughput_controller();
    }
}

ReturnCode_t DataWriterImpl::check_qos(
        const DataWriterQos& qos)
{
    if (qos.durability().kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(RTPS_QOS_CHECK, "PERSISTENT Durability not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (qos.destination_order().kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(RTPS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return ReturnCode_t::RETCODE_UNSUPPORTED;
    }
    if (qos.reliability().kind == BEST_EFFORT_RELIABILITY_QOS && qos.ownership().kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(RTPS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
    }
    if (qos.liveliness().kind == AUTOMATIC_LIVELINESS_QOS ||
            qos.liveliness().kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (qos.liveliness().lease_duration < eprosima::fastrtps::c_TimeInfinite &&
                qos.liveliness().lease_duration <= qos.liveliness().announcement_period)
        {
            logError(RTPS_QOS_CHECK, "WRITERQOS: LeaseDuration <= announcement period.");
            return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

bool DataWriterImpl::can_qos_be_updated(
        const DataWriterQos& to,
        const DataWriterQos& from)
{
    bool updatable = true;
    if (to.durability().kind != from.durability().kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().kind !=  from.liveliness().kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().lease_duration != from.liveliness().lease_duration)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness lease duration cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().announcement_period != from.liveliness().announcement_period)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness announcement cannot be changed after the creation of a DataWriter.");
    }

    if (to.reliability().kind != from.reliability().kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a DataWriter.");
    }
    if (to.ownership().kind != from.ownership().kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a DataWriter.");
    }
    if (to.destination_order().kind != from.destination_order().kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a DataWriter.");
    }
    return updatable;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
