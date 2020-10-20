// Copyright 2019, 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fastrtps/config.h>

#include <fastdds/publisher/DataWriterImpl.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>

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
#include <fastdds/core/policy/ParameterSerializer.hpp>

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
    , type_(type)
    , topic_(topic)
    , qos_(&qos == &DATAWRITER_QOS_DEFAULT ? publisher_->get_default_datawriter_qos() : qos)
    , history_(get_topic_attributes(qos_, *topic_, type_), type_->m_typeSize
#if HAVE_SECURITY
            // In future v2 changepool is in writer, and writer set this value to cachechagepool.
            + 20 /*SecureDataHeader*/ + 4 + ((2 * 16) /*EVP_MAX_IV_LENGTH max block size*/ - 1 ) /* SecureDataBodey*/
            + 16 + 4 /*SecureDataTag*/
#endif // if HAVE_SECURITY
            , qos_.endpoint().history_memory_policy)
    , listener_(listen)
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , high_mark_for_frag_(0)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
{
}

ReturnCode_t DataWriterImpl::enable()
{
    assert(writer_ == nullptr);

    WriterAttributes w_att;
    w_att.throughputController = qos_.throughput_controller();
    w_att.endpoint.durabilityKind = qos_.durability().durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    w_att.endpoint.multicastLocatorList = qos_.endpoint().multicast_locator_list;
    w_att.endpoint.reliabilityKind = qos_.reliability().kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = type_->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    w_att.endpoint.unicastLocatorList = qos_.endpoint().unicast_locator_list;
    w_att.endpoint.remoteLocatorList = qos_.endpoint().remote_locator_list;
    w_att.mode = qos_.publish_mode().kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
    w_att.endpoint.properties = qos_.properties();

    if (qos_.endpoint().entity_id > 0)
    {
        w_att.endpoint.setEntityID(static_cast<uint8_t>(qos_.endpoint().entity_id));
    }

    if (qos_.endpoint().user_defined_id > 0)
    {
        w_att.endpoint.setUserDefinedID(static_cast<uint8_t>(qos_.endpoint().user_defined_id));
    }

    w_att.times = qos_.reliable_writer_qos().times;
    w_att.liveliness_kind = qos_.liveliness().kind;
    w_att.liveliness_lease_duration = qos_.liveliness().lease_duration;
    w_att.liveliness_announcement_period = qos_.liveliness().announcement_period;
    w_att.matched_readers_allocation = qos_.writer_resource_limits().matched_subscriber_allocation;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_->get_name().c_str());
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

    if (qos_.reliable_writer_qos().disable_positive_acks.enabled &&
            qos_.reliable_writer_qos().disable_positive_acks.duration != c_TimeInfinite)
    {
        w_att.disable_positive_acks = true;
        w_att.keep_duration = qos_.reliable_writer_qos().disable_positive_acks.duration;
    }

    RTPSWriter* writer = RTPSDomain::createRTPSWriter(
        publisher_->rtps_participant(),
        w_att,
        static_cast<WriterHistory*>(&history_),
        static_cast<WriterListener*>(&writer_listener_));

    if (writer == nullptr)
    {
        logError(DATA_WRITER, "Problem creating associated Writer");
        return ReturnCode_t::RETCODE_ERROR;
    }

    writer_ = writer;

    // In case it has been loaded from the persistence DB, rebuild instances on history
    history_.rebuild_instances();

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

    for (PublisherHistory::iterator it = history_.changesBegin(); it != history_.changesEnd(); it++)
    {
        WriteParams wparams;
        set_fragment_size_on_change(wparams, *it, high_mark_for_frag_);
    }

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

    // REGISTER THE WRITER
    WriterQos wqos = qos_.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
    publisher_->rtps_participant()->registerWriter(writer_, get_topic_attributes(qos_, *topic_, type_), wqos);

    return ReturnCode_t::RETCODE_OK;
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
        RTPSDomain::removeRTPSWriter(writer_);
    }

    delete user_datawriter_;
}

bool DataWriterImpl::write(
        void* data)
{
    if (writer_ == nullptr)
    {
        return false;
    }

    logInfo(DATA_WRITER, "Writing new data");
    return create_new_change(ALIVE, data);
}

bool DataWriterImpl::write(
        void* data,
        fastrtps::rtps::WriteParams& params)
{
    if (writer_ == nullptr)
    {
        return false;
    }

    logInfo(DATA_WRITER, "Writing new data with WriteParams");
    return create_new_change_with_params(ALIVE, data, params);
}

ReturnCode_t DataWriterImpl::write(
        void* data,
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    InstanceHandle_t instance_handle;
    if (type_.get()->m_isGetKeyDefined)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        type_.get()->getKey(data, &instance_handle, is_key_protected);
    }

    //Check if the Handle is different from the special value HANDLE_NIL and
    //does not correspond with the instance referred by the data
    if (handle.isDefined() && handle.value != instance_handle.value)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }
    logInfo(DATA_WRITER, "Writing new data with Handle");
    WriteParams wparams;
    if (create_new_change_with_params(ALIVE, data, wparams, instance_handle))
    {
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

fastrtps::rtps::InstanceHandle_t DataWriterImpl::register_instance(
        void* key)
{
    /// Preconditions
    if (writer_ == nullptr)
    {
        return c_InstanceHandle_Unknown;
    }

    if (key == nullptr)
    {
        logError(PUBLISHER, "Data pointer not valid");
        return c_InstanceHandle_Unknown;
    }

    if (!type_->m_isGetKeyDefined)
    {
        logError(PUBLISHER, "Topic is NO_KEY, operation not permitted");
        return c_InstanceHandle_Unknown;
    }

    InstanceHandle_t instance_handle = c_InstanceHandle_Unknown;
    bool is_key_protected = false;
#if HAVE_SECURITY
    is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
    type_->getKey(key, &instance_handle, is_key_protected);

    // Block lowlevel writer
    auto max_blocking_time = std::chrono::steady_clock::now() +
            std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (lock.try_lock_until(max_blocking_time))
#else
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // if HAVE_STRICT_REALTIME
    {
        if (history_.register_instance(instance_handle, lock, max_blocking_time))
        {
            return instance_handle;
        }
    }

    return c_InstanceHandle_Unknown;
}

ReturnCode_t DataWriterImpl::unregister_instance(
        void* instance,
        const InstanceHandle_t& handle,
        bool dispose)
{
    /// Preconditions
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    if (instance == nullptr)
    {
        logError(PUBLISHER, "Data pointer not valid");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    if (!type_->m_isGetKeyDefined)
    {
        logError(PUBLISHER, "Topic is NO_KEY, operation not permitted");
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    ReturnCode_t returned_value = ReturnCode_t::RETCODE_ERROR;
    InstanceHandle_t ih = handle;

#if !defined(NDEBUG)
    if (c_InstanceHandle_Unknown == ih)
#endif // if !defined(NDEBUG)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        type_->getKey(instance, &ih, is_key_protected);
    }

#if !defined(NDEBUG)
    if (c_InstanceHandle_Unknown != handle && ih != handle)
    {
        logError(PUBLISHER, "handle differs from data's key.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
#endif // if !defined(NDEBUG)

    if (history_.is_key_registered(ih))
    {
        WriteParams wparams;
        ChangeKind_t change_kind = NOT_ALIVE_DISPOSED;
        if (!dispose)
        {
            change_kind = qos_.writer_data_lifecycle().autodispose_unregistered_instances ?
                    NOT_ALIVE_DISPOSED_UNREGISTERED :
                    NOT_ALIVE_UNREGISTERED;
        }

        if (create_new_change_with_params(change_kind, instance, wparams, ih))
        {
            returned_value = ReturnCode_t::RETCODE_OK;
        }
    }
    else
    {
        returned_value = ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    return returned_value;
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
#endif // if HAVE_STRICT_REALTIME
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

            set_fragment_size_on_change(wparams, ch, high_mark_for_frag_);

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
                lifespan_duration_us_ = duration<double, std::ratio<1, 1000000>>(
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
#endif // if HAVE_SECURITY
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

ReturnCode_t DataWriterImpl::clear_history(
        size_t* removed)
{
    return (history_.removeAllChange(removed) ? ReturnCode_t::RETCODE_OK : ReturnCode_t::RETCODE_ERROR);
}

const GUID_t& DataWriterImpl::guid() const
{
    return writer_ ? writer_->getGuid() : c_Guid_Unknown;
}

InstanceHandle_t DataWriterImpl::get_instance_handle() const
{
    return guid();
}

void DataWriterImpl::publisher_qos_updated()
{
    if (writer_ != nullptr)
    {
        //NOTIFY THE BUILTIN PROTOCOLS THAT THE WRITER HAS CHANGED
        WriterQos wqos = qos_.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
        publisher_->rtps_participant()->updateWriter(writer_, get_topic_attributes(qos_, *topic_, type_), wqos);
    }
}

ReturnCode_t DataWriterImpl::set_qos(
        const DataWriterQos& qos)
{
    bool enabled = writer_ != nullptr;
    const DataWriterQos& qos_to_set = (&qos == &DATAWRITER_QOS_DEFAULT) ?
            publisher_->get_default_datawriter_qos() : qos;

    // Default qos is always considered consistent
    if (&qos != &DATAWRITER_QOS_DEFAULT)
    {
        ReturnCode_t ret_val = check_qos(qos_to_set);
        if (!ret_val)
        {
            return ret_val;
        }

        if (publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data != 0 &&
                publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data <
                qos_to_set.user_data().getValue().size())
        {
            return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
        }
    }

    if (enabled && !can_qos_be_updated(qos_, qos_to_set))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }
    set_qos(qos_, qos_to_set, !enabled);

    if (enabled)
    {
        //Notify the participant that a Writer has changed its QOS
        fastrtps::TopicAttributes topic_att = get_topic_attributes(qos_, *topic_, type_);
        WriterQos wqos = qos_.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
        publisher_->rtps_participant()->updateWriter(writer_, topic_att, wqos);

        // Deadline
        if (qos_.deadline().period != c_TimeInfinite)
        {
            deadline_duration_us_ =
                    duration<double, std::ratio<1, 1000000>>(qos_.deadline().period.to_ns() * 1e-3);
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
                    duration<double, std::ratio<1, 1000000>>(qos_.lifespan().duration.to_ns() * 1e-3);
            lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
        }
        else
        {
            lifespan_timer_->cancel_timer();
        }
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
    DataWriterListener* listener = data_writer_->get_listener_for(StatusMask::publication_matched());
    if (listener != nullptr)
    {
        listener->on_publication_matched(data_writer_->user_datawriter_, info);
    }
}

void DataWriterImpl::InnerDataWriterListener::on_offered_incompatible_qos(
        RTPSWriter* /*writer*/,
        fastdds::dds::PolicyMask qos)
{
    data_writer_->update_offered_incompatible_qos(qos);
    DataWriterListener* listener = data_writer_->get_listener_for(StatusMask::offered_incompatible_qos());
    if (listener != nullptr)
    {
        OfferedIncompatibleQosStatus callback_status;
        if (data_writer_->get_offered_incompatible_qos_status(callback_status) == ReturnCode_t::RETCODE_OK)
        {
            listener->on_offered_incompatible_qos(data_writer_->user_datawriter_, callback_status);
        }
    }
}

void DataWriterImpl::InnerDataWriterListener::onWriterChangeReceivedByAll(
        RTPSWriter* /*writer*/,
        CacheChange_t* ch)
{
    if (data_writer_->type_->m_isGetKeyDefined &&
            (NOT_ALIVE_UNREGISTERED == ch->kind ||
            NOT_ALIVE_DISPOSED_UNREGISTERED == ch->kind))
    {
        data_writer_->history_.remove_instance_changes(ch->instanceHandle, ch->sequenceNumber);
    }
    else if (data_writer_->qos_.durability().kind == VOLATILE_DURABILITY_QOS)
    {
        data_writer_->history_.remove_change_g(ch);
    }
}

void DataWriterImpl::InnerDataWriterListener::on_liveliness_lost(
        fastrtps::rtps::RTPSWriter* /*writer*/,
        const fastrtps::LivelinessLostStatus& status)
{
    DataWriterListener* listener = data_writer_->get_listener_for(StatusMask::liveliness_lost());
    if (listener != nullptr)
    {
        listener->on_liveliness_lost(
            data_writer_->user_datawriter_, status);
    }
}

ReturnCode_t DataWriterImpl::wait_for_acknowledgments(
        const Duration_t& max_wait)
{
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

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
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    status = deadline_missed_status_;
    deadline_missed_status_.total_count_change = 0;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriterImpl::get_offered_incompatible_qos_status(
        OfferedIncompatibleQosStatus& status)
{
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    status = offered_incompatible_qos_status_;
    offered_incompatible_qos_status_.total_count_change = 0u;
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
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    status.total_count = writer_->liveliness_lost_status_.total_count;
    status.total_count_change = writer_->liveliness_lost_status_.total_count_change;

    writer_->liveliness_lost_status_.total_count_change = 0u;

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DataWriterImpl::assert_liveliness()
{
    if (writer_ == nullptr)
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }

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

OfferedIncompatibleQosStatus& DataWriterImpl::update_offered_incompatible_qos(
        PolicyMask incompatible_policies)
{
    ++offered_incompatible_qos_status_.total_count;
    ++offered_incompatible_qos_status_.total_count_change;
    for (uint32_t id = 1; id < NEXT_QOS_POLICY_ID; ++id)
    {
        if (incompatible_policies.test(id))
        {
            ++offered_incompatible_qos_status_.policies[static_cast<QosPolicyId_t>(id)].count;
            offered_incompatible_qos_status_.last_policy_id = static_cast<QosPolicyId_t>(id);
        }
    }
    return offered_incompatible_qos_status_;
}

void DataWriterImpl::set_qos(
        DataWriterQos& to,
        const DataWriterQos& from,
        bool is_default)
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
    if (!(to.user_data() == from.user_data()))
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
    if (is_default && !(to.reliable_writer_qos() == from.reliable_writer_qos()))
    {
        to.reliable_writer_qos() = from.reliable_writer_qos();
    }
    if (is_default && !(to.endpoint() == from.endpoint()))
    {
        to.endpoint() = from.endpoint();
    }
    if (is_default && !(to.writer_resource_limits() == from.writer_resource_limits()))
    {
        to.writer_resource_limits() = from.writer_resource_limits();
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
    if (qos.data_sharing().kind() == DataSharingKind::FORCED &&
            (qos.endpoint().history_memory_policy != PREALLOCATED_MEMORY_MODE &&
            qos.endpoint().history_memory_policy != PREALLOCATED_WITH_REALLOC_MEMORY_MODE))
    {
        return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
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

DataWriterListener* DataWriterImpl::get_listener_for(
        const StatusMask& status)
{
    if (listener_ != nullptr &&
            user_datawriter_->get_status_mask().is_active(status))
    {
        return listener_;
    }
    return publisher_->get_listener_for(status);
}

void DataWriterImpl::set_fragment_size_on_change(
        WriteParams& wparams,
        CacheChange_t* ch,
        const uint32_t& high_mark_for_frag)
{
    uint32_t final_high_mark_for_frag = high_mark_for_frag;

    // If needed inlineqos for related_sample_identity, then remove the inlinqos size from final fragment size.
    if (wparams.related_sample_identity() != SampleIdentity::unknown())
    {
        final_high_mark_for_frag -= (
            ParameterSerializer<Parameter_t>::PARAMETER_SENTINEL_SIZE +
            ParameterSerializer<Parameter_t>::PARAMETER_SAMPLE_IDENTITY_SIZE);
    }

    // If it is big data, fragment it.
    if (ch->serializedPayload.length > final_high_mark_for_frag)
    {
        // Fragment the data.
        // Set the fragment size to the cachechange.
        ch->setFragmentSize(static_cast<uint16_t>(
                    (std::min)(final_high_mark_for_frag, RTPSMessageGroup::get_max_fragment_payload_size())));
    }
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
