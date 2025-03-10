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
#include <fastdds/publisher/DataWriterImpl.hpp>

#include <functional>
#include <iostream>

#include <fastdds/config.hpp>
#include <fastdds/core/condition/StatusConditionImpl.hpp>
#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/core/policy/QosPolicyUtils.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/publisher/filtering/DataWriterFilteredChangePool.hpp>
#include <fastdds/publisher/PublisherImpl.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <fastdds/utils/TypePropagation.hpp>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/DataSharing/DataSharingPayloadPool.hpp>
#include <rtps/DataSharing/WriterPool.hpp>
#include <rtps/history/CacheChangePool.h>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/resources/TimedEvent.h>
#include <rtps/RTPSDomainImpl.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/StatefulWriter.hpp>
#include <utils/TimeConversion.hpp>
#include <utils/BuiltinTopicKeyConversions.hpp>
#ifdef FASTDDS_STATISTICS
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/types/monitorservice_types.hpp>
#endif //FASTDDS_STATISTICS

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace std::chrono;

namespace eprosima {
namespace fastdds {
namespace dds {

static ChangeKind_t unregister_change_kind(
        bool dispose,
        const DataWriterQos& qos)
{
    if (dispose)
    {
        return NOT_ALIVE_DISPOSED;
    }

    return qos.writer_data_lifecycle().autodispose_unregistered_instances ?
           NOT_ALIVE_DISPOSED_UNREGISTERED : NOT_ALIVE_UNREGISTERED;
}

static bool qos_has_pull_mode_request(
        const DataWriterQos& qos)
{
    auto push_mode = PropertyPolicyHelper::find_property(qos.properties(), "fastdds.push_mode");
    return (nullptr != push_mode) && ("false" == *push_mode);
}

class DataWriterImpl::LoanCollection
{
public:

    explicit LoanCollection(
            const PoolConfig& config)
        : loans_(get_collection_limits(config))
    {
    }

    bool add_loan(
            const void* const data,
            SerializedPayload_t& payload)
    {
        static_cast<void>(data);
        assert(data == payload.data + SerializedPayload_t::representation_header_size);
        return loans_.push_back(std::move(payload));
    }

    bool check_and_remove_loan(
            const void* const data,
            SerializedPayload_t& payload)
    {
        const octet* payload_data = static_cast<const octet*>(data) - SerializedPayload_t::representation_header_size;
        for (auto it = loans_.begin(); it != loans_.end(); ++it)
        {
            if (it->data == payload_data)
            {
                // Avoid releasing the payload in destructor
                payload = std::move(*it);
                loans_.erase(it);
                return true;
            }
        }
        return false;
    }

    bool is_empty() const
    {
        return loans_.empty();
    }

private:

    static ResourceLimitedContainerConfig get_collection_limits(
            const PoolConfig& config)
    {
        return
            {
                config.initial_size,
                config.maximum_size,
                config.initial_size == config.maximum_size ? 0u : 1u
            };
    }

    ResourceLimitedVector<SerializedPayload_t> loans_;

};

DataWriterImpl::DataWriterImpl(
        PublisherImpl* p,
        TypeSupport type,
        Topic* topic,
        const DataWriterQos& qos,
        DataWriterListener* listen,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
    : publisher_(p)
    , type_(type)
    , topic_(topic)
    , qos_(get_datawriter_qos_from_settings(qos))
    , listener_(listen)
    , history_()
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
{
    EndpointAttributes endpoint_attributes;
    endpoint_attributes.endpointKind = WRITER;
    endpoint_attributes.topicKind = type_->is_compute_key_provided ? WITH_KEY : NO_KEY;
    endpoint_attributes.setEntityID(qos_.endpoint().entity_id);
    endpoint_attributes.setUserDefinedID(qos_.endpoint().user_defined_id);
    fastdds::rtps::RTPSParticipantImpl::preprocess_endpoint_attributes<WRITER, 0x03, 0x02>(
        fastdds::rtps::EntityId_t::unknown(),
        publisher_->get_participant_impl()->id_counter(), endpoint_attributes, guid_.entityId);
    guid_.guidPrefix = publisher_->get_participant_impl()->guid().guidPrefix;

    if (payload_pool != nullptr)
    {
        is_custom_payload_pool_ = true;
        payload_pool_ = payload_pool;
    }
}

DataWriterImpl::DataWriterImpl(
        PublisherImpl* p,
        TypeSupport type,
        Topic* topic,
        const DataWriterQos& qos,
        const fastdds::rtps::EntityId_t& entity_id,
        DataWriterListener* listen)
    : publisher_(p)
    , type_(type)
    , topic_(topic)
    , qos_(get_datawriter_qos_from_settings(qos))
    , listener_(listen)
    , history_()
#pragma warning (disable : 4355 )
    , writer_listener_(this)
    , deadline_duration_us_(qos_.deadline().period.to_ns() * 1e-3)
    , lifespan_duration_us_(qos_.lifespan().duration.to_ns() * 1e-3)
{
    guid_ = { publisher_->get_participant_impl()->guid().guidPrefix, entity_id};
}

DataWriterQos DataWriterImpl::get_datawriter_qos_from_settings(
        const DataWriterQos& qos)
{
    DataWriterQos return_qos;
    if (&DATAWRITER_QOS_DEFAULT == &qos)
    {
        return_qos = publisher_->get_default_datawriter_qos();
    }
    else if (&DATAWRITER_QOS_USE_TOPIC_QOS == &qos)
    {
        return_qos = publisher_->get_default_datawriter_qos();
        publisher_->copy_from_topic_qos(return_qos, topic_->get_qos());
    }
    else
    {
        return_qos = qos;
    }

    return return_qos;
}

void DataWriterImpl::create_history(
        const std::shared_ptr<IPayloadPool>& payload_pool,
        const std::shared_ptr<IChangePool>& change_pool)
{
    history_.reset(new DataWriterHistory(
                payload_pool, change_pool,
                qos_.history(),
                qos_.resource_limits(),
                (type_->is_compute_key_provided ? WITH_KEY : NO_KEY),
                type_->max_serialized_type_size,
                qos_.endpoint().history_memory_policy,
                [this](
                    const InstanceHandle_t& handle) -> void
                {
                    if (nullptr != listener_)
                    {
                        listener_->on_unacknowledged_sample_removed(user_datawriter_, handle);
                    }
                }));
}

ReturnCode_t DataWriterImpl::enable()
{
    assert(writer_ == nullptr);

    auto history_att = DataWriterHistory::to_history_attributes(
        qos_.history(),
        qos_.resource_limits(), (type_->is_compute_key_provided ? WITH_KEY : NO_KEY), type_->max_serialized_type_size,
        qos_.endpoint().history_memory_policy);
    pool_config_ = PoolConfig::from_history_attributes(history_att);

    // When the user requested PREALLOCATED_WITH_REALLOC, but we know the type cannot
    // grow, we translate the policy into bare PREALLOCATED
    if (PREALLOCATED_WITH_REALLOC_MEMORY_MODE == pool_config_.memory_policy &&
            (type_->is_bounded() || type_->is_plain(data_representation_)))
    {
        pool_config_.memory_policy = PREALLOCATED_MEMORY_MODE;
    }

    WriterAttributes w_att;
    w_att.endpoint.durabilityKind = qos_.durability().durabilityKind();
    w_att.endpoint.endpointKind = WRITER;
    w_att.endpoint.reliabilityKind = qos_.reliability().kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    w_att.endpoint.topicKind = type_->is_compute_key_provided ? WITH_KEY : NO_KEY;
    w_att.endpoint.multicastLocatorList = qos_.endpoint().multicast_locator_list;
    w_att.endpoint.unicastLocatorList = qos_.endpoint().unicast_locator_list;
    w_att.endpoint.remoteLocatorList = qos_.endpoint().remote_locator_list;
    w_att.endpoint.external_unicast_locators = qos_.endpoint().external_unicast_locators;
    w_att.endpoint.ignore_non_matching_locators = qos_.endpoint().ignore_non_matching_locators;
    w_att.mode = qos_.publish_mode().kind == SYNCHRONOUS_PUBLISH_MODE ? SYNCHRONOUS_WRITER : ASYNCHRONOUS_WRITER;
    w_att.flow_controller_name = qos_.publish_mode().flow_controller_name;
    w_att.endpoint.properties = qos_.properties();
    w_att.endpoint.ownershipKind = qos_.ownership().kind;
    w_att.endpoint.setEntityID(qos_.endpoint().entity_id);
    w_att.endpoint.setUserDefinedID(qos_.endpoint().user_defined_id);
    w_att.times = qos_.reliable_writer_qos().times;
    w_att.liveliness_kind = qos_.liveliness().kind;
    w_att.liveliness_lease_duration = qos_.liveliness().lease_duration;
    w_att.liveliness_announcement_period = qos_.liveliness().announcement_period;
    w_att.matched_readers_allocation = qos_.writer_resource_limits().matched_subscriber_allocation;
    w_att.disable_heartbeat_piggyback = qos_.reliable_writer_qos().disable_heartbeat_piggyback;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_->get_name().c_str());
    w_att.endpoint.properties.properties().push_back(std::move(property));

    std::string* endpoint_partitions = PropertyPolicyHelper::find_property(qos_.properties(), "partitions");

    if (endpoint_partitions)
    {
        property.name("partitions");
        property.value(*endpoint_partitions);
        w_att.endpoint.properties.properties().push_back(std::move(property));
    }
    else if (publisher_->get_qos().partition().names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        bool is_first_partition = true;
        for (auto partition : publisher_->get_qos().partition().names())
        {
            partitions += (is_first_partition ? "" : ";") + partition;
            is_first_partition = false;
        }
        property.value(std::move(partitions));
        w_att.endpoint.properties.properties().push_back(std::move(property));
    }

    if (qos_.reliable_writer_qos().disable_positive_acks.enabled &&
            qos_.reliable_writer_qos().disable_positive_acks.duration != dds::c_TimeInfinite)
    {
        w_att.disable_positive_acks = true;
        w_att.keep_duration = qos_.reliable_writer_qos().disable_positive_acks.duration;
    }

    ReturnCode_t ret_code = check_datasharing_compatible(w_att, is_data_sharing_compatible_);
    if (ret_code != RETCODE_OK)
    {
        return ret_code;
    }

    if (is_data_sharing_compatible_)
    {
        DataSharingQosPolicy datasharing(qos_.data_sharing());
        if (datasharing.domain_ids().empty())
        {
            datasharing.add_domain_id(utils::default_domain_id());
        }
        w_att.endpoint.set_data_sharing_configuration(datasharing);
    }
    else
    {
        DataSharingQosPolicy datasharing;
        datasharing.off();
        w_att.endpoint.set_data_sharing_configuration(datasharing);
    }

    bool filtering_enabled =
            qos_.liveliness().lease_duration.is_infinite() &&
            (0 < qos_.writer_resource_limits().reader_filters_allocation.maximum);
    if (filtering_enabled)
    {
        reader_filters_.reset(new ReaderFilterCollection(qos_.writer_resource_limits().reader_filters_allocation));
    }

    // Set Datawriter's DataRepresentationId taking into account the QoS.
    data_representation_ = qos_.representation().m_value.empty()
            || XCDR_DATA_REPRESENTATION == qos_.representation().m_value.at(0)
                    ? XCDR_DATA_REPRESENTATION : XCDR2_DATA_REPRESENTATION;

    auto change_pool = get_change_pool();
    if (!change_pool)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Problem creating change pool for associated Writer");
        return RETCODE_ERROR;
    }

    auto pool = get_payload_pool();
    if (!pool)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Problem creating payload pool for associated Writer");
        return RETCODE_ERROR;
    }

    create_history(pool, change_pool);

    RTPSWriter* writer =  RTPSDomainImpl::create_rtps_writer(
        publisher_->rtps_participant(),
        guid_.entityId,
        w_att,
        history_.get(),
        static_cast<WriterListener*>(&writer_listener_));

    if (writer != nullptr &&
            w_att.endpoint.data_sharing_configuration().kind() != DataSharingKind::OFF)
    {
        auto writer_pool = std::dynamic_pointer_cast<fastdds::rtps::WriterPool>(pool);
        if (!writer_pool || !writer_pool->is_initialized())
        {
            EPROSIMA_LOG_ERROR(DATA_WRITER, "Could not initialize DataSharing writer pool");
            RTPSDomain::removeRTPSWriter(writer);
            writer = nullptr;
        }
    }

    if (writer == nullptr &&
            w_att.endpoint.data_sharing_configuration().kind() == DataSharingKind::AUTO)
    {
        EPROSIMA_LOG_INFO(DATA_WRITER, "Trying with a non-datasharing pool");
        history_.reset();
        release_payload_pool();
        is_data_sharing_compatible_ = false;
        DataSharingQosPolicy datasharing;
        datasharing.off();
        w_att.endpoint.set_data_sharing_configuration(datasharing);

        pool = get_payload_pool();
        if (!pool)
        {
            EPROSIMA_LOG_ERROR(DATA_WRITER, "Problem creating payload pool for associated Writer");
            return RETCODE_ERROR;
        }

        create_history(pool, change_pool);
        writer = RTPSDomainImpl::create_rtps_writer(
            publisher_->rtps_participant(),
            guid_.entityId,
            w_att,
            history_.get(),
            static_cast<WriterListener*>(&writer_listener_));
    }
    if (writer == nullptr)
    {
        history_.reset();
        release_payload_pool();
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Problem creating associated Writer");
        return RETCODE_ERROR;
    }

    writer_ = BaseWriter::downcast(writer);
    if (filtering_enabled)
    {
        writer_->reader_data_filter(this);
    }

    // In case it has been loaded from the persistence DB, rebuild instances on history
    history_->rebuild_instances();

    deadline_timer_ = new TimedEvent(publisher_->rtps_participant()->get_resource_event(),
                    [&]() -> bool
                    {
                        return deadline_missed();
                    },
                    qos_.deadline().period.to_ns() * 1e-6);

    lifespan_timer_ = new TimedEvent(publisher_->rtps_participant()->get_resource_event(),
                    [&]() -> bool
                    {
                        return lifespan_expired();
                    },
                    qos_.lifespan().duration.to_ns() * 1e-6);

    // In case it has been loaded from the persistence DB, expire old samples.
    if (qos_.lifespan().duration != dds::c_TimeInfinite)
    {
        if (lifespan_expired())
        {
            lifespan_timer_->restart_timer();
        }
    }

    // REGISTER THE WRITER
    fastdds::rtps::TopicDescription topic_desc;
    topic_desc.topic_name = topic_->get_name();
    topic_desc.type_name = topic_->get_type_name();
    publisher_->get_participant_impl()->fill_type_information(type_, topic_desc.type_information);

    WriterQos wqos = qos_.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
    if (!is_data_sharing_compatible_)
    {
        wqos.data_sharing.off();
    }
    if (endpoint_partitions)
    {
        std::istringstream partition_string(*endpoint_partitions);
        std::string partition_name;
        wqos.m_partition.clear();

        while (std::getline(partition_string, partition_name, ';'))
        {
            wqos.m_partition.push_back(partition_name.c_str());
        }
    }
    publisher_->rtps_participant()->register_writer(writer_, topic_desc, wqos);

    return RETCODE_OK;
}

void DataWriterImpl::disable()
{
    set_listener(nullptr);
    if (writer_ != nullptr)
    {
        writer_->set_listener(nullptr);
    }
}

ReturnCode_t DataWriterImpl::check_delete_preconditions()
{
    if (loans_ && !loans_->is_empty())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return RETCODE_OK;
}

DataWriterImpl::~DataWriterImpl()
{
    delete lifespan_timer_;
    delete deadline_timer_;

    if (writer_ != nullptr)
    {
        EPROSIMA_LOG_INFO(DATA_WRITER, guid().entityId << " in topic: " << type_->get_name());
        RTPSDomain::removeRTPSWriter(writer_);
        release_payload_pool();
    }

    delete user_datawriter_;
}

ReturnCode_t DataWriterImpl::loan_sample(
        void*& sample,
        LoanInitializationKind initialization)
{
    // Block lowlevel writer
    auto max_blocking_time = steady_clock::now() +
            microseconds(rtps::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));

    // Type should be plain and have space for the representation header
    if (!type_->is_plain(data_representation_) ||
            SerializedPayload_t::representation_header_size > type_->max_serialized_type_size)
    {
        return RETCODE_ILLEGAL_OPERATION;
    }

    // Writer should be enabled
    if (nullptr == writer_)
    {
        return RETCODE_NOT_ENABLED;
    }

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        return RETCODE_TIMEOUT;
    }
#else
    static_cast<void>(max_blocking_time);
    std::lock_guard<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // if HAVE_STRICT_REALTIME

    // Get one payload from the pool
    SerializedPayload_t payload;
    uint32_t size = type_->max_serialized_type_size;
    if (!get_free_payload_from_pool(size, payload))
    {
        return RETCODE_OUT_OF_RESOURCES;
    }

    // Leave payload state as if serialization has already been performed
    payload.length = size;
    payload.pos = size;
    payload.data[1] = DEFAULT_ENCAPSULATION;
    payload.encapsulation = DEFAULT_ENCAPSULATION;

    // Sample starts after representation header
    sample = payload.data + SerializedPayload_t::representation_header_size;

    // Add to loans collection
    if (!add_loan(sample, payload))
    {
        sample = nullptr;
        payload_pool_->release_payload(payload);
        return RETCODE_OUT_OF_RESOURCES;
    }

    switch (initialization)
    {
        default:
            EPROSIMA_LOG_WARNING(DATA_WRITER, "Using wrong LoanInitializationKind value ("
                    << static_cast<int>(initialization) << "). Using default NO_LOAN_INITIALIZATION");
            break;

        case LoanInitializationKind::NO_LOAN_INITIALIZATION:
            break;

        case LoanInitializationKind::ZERO_LOAN_INITIALIZATION:
            if (SerializedPayload_t::representation_header_size < size)
            {
                size -= SerializedPayload_t::representation_header_size;
                memset(sample, 0, size);
            }
            break;

        case LoanInitializationKind::CONSTRUCTED_LOAN_INITIALIZATION:
            if (!type_->construct_sample(sample))
            {
                check_and_remove_loan(sample, payload);
                payload_pool_->release_payload(payload);
                sample = nullptr;
                return RETCODE_UNSUPPORTED;
            }
            break;
    }

    // Avoid releasing the payload in destructor
    payload.payload_owner = nullptr;
    payload.data = nullptr;

    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::discard_loan(
        void*& sample)
{
    // Type should be plain and have space for the representation header
    if (!type_->is_plain(data_representation_) ||
            SerializedPayload_t::representation_header_size > type_->max_serialized_type_size)
    {
        return RETCODE_ILLEGAL_OPERATION;
    }

    // Writer should be enabled
    if (nullptr == writer_)
    {
        return RETCODE_NOT_ENABLED;
    }

    std::lock_guard<RecursiveTimedMutex> lock(writer_->getMutex());

    // Remove sample from loans collection
    SerializedPayload_t payload;
    if ((nullptr == sample) || !check_and_remove_loan(sample, payload))
    {
        return RETCODE_BAD_PARAMETER;
    }

    // Return payload to pool
    payload_pool_->release_payload(payload);
    sample = nullptr;

    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::write(
        const void* const data)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    EPROSIMA_LOG_INFO(DATA_WRITER, "Writing new data");
    return create_new_change(ALIVE, data);
}

ReturnCode_t DataWriterImpl::write(
        const void* const data,
        fastdds::rtps::WriteParams& params)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    EPROSIMA_LOG_INFO(DATA_WRITER, "Writing new data with WriteParams");
    return create_new_change_with_params(ALIVE, data, params);
}

ReturnCode_t DataWriterImpl::check_write_preconditions(
        const void* const data,
        const InstanceHandle_t& handle,
        InstanceHandle_t& instance_handle)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    if (type_.get()->is_compute_key_provided)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        type_.get()->compute_key(data, instance_handle, is_key_protected);
    }

    //Check if the Handle is different from the special value HANDLE_NIL and
    //does not correspond with the instance referred by the data
    if (handle.isDefined() && handle != instance_handle)
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::write(
        const void* const data,
        const InstanceHandle_t& handle)
{
    InstanceHandle_t instance_handle;
    ReturnCode_t ret = check_write_preconditions(data, handle, instance_handle);
    if (RETCODE_OK == ret)
    {
        EPROSIMA_LOG_INFO(DATA_WRITER, "Writing new data with Handle");
        WriteParams wparams;
        ret = create_new_change_with_params(ALIVE, data, wparams, instance_handle);
    }

    return ret;
}

ReturnCode_t DataWriterImpl::write_w_timestamp(
        const void* const data,
        const InstanceHandle_t& handle,
        const fastdds::dds::Time_t& timestamp)
{
    InstanceHandle_t instance_handle;
    ReturnCode_t ret = RETCODE_OK;
    if (timestamp.is_infinite() || timestamp.seconds < 0)
    {
        ret = RETCODE_BAD_PARAMETER;
    }

    if (RETCODE_OK == ret)
    {
        ret = check_write_preconditions(data, handle, instance_handle);
    }

    if (RETCODE_OK == ret)
    {
        EPROSIMA_LOG_INFO(DATA_WRITER, "Writing new data with Handle and timestamp");
        WriteParams wparams;
        wparams.source_timestamp(timestamp);
        ret = create_new_change_with_params(ALIVE, data, wparams, instance_handle);
    }

    return ret;
}

ReturnCode_t DataWriterImpl::check_instance_preconditions(
        const void* const data,
        const InstanceHandle_t& handle,
        InstanceHandle_t& instance_handle)
{
    if (nullptr == writer_)
    {
        return RETCODE_NOT_ENABLED;
    }

    if (nullptr == data)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Data pointer not valid");
        return RETCODE_BAD_PARAMETER;
    }

    if (!type_->is_compute_key_provided)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Topic is NO_KEY, operation not permitted");
        return RETCODE_PRECONDITION_NOT_MET;
    }

    instance_handle = handle;

#if defined(NDEBUG)
    if (!instance_handle.isDefined())
#endif // if !defined(NDEBUG)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        type_->compute_key(data, instance_handle, is_key_protected);
    }

#if !defined(NDEBUG)
    if (handle.isDefined() && instance_handle != handle)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "handle differs from data's key.");
        return RETCODE_PRECONDITION_NOT_MET;
    }
#endif // if !defined(NDEBUG)

    return RETCODE_OK;
}

InstanceHandle_t DataWriterImpl::register_instance(
        const void* const key)
{
    /// Preconditions
    InstanceHandle_t instance_handle;
    if (RETCODE_OK != check_instance_preconditions(key, HANDLE_NIL, instance_handle))
    {
        return HANDLE_NIL;
    }

    WriteParams wparams;
    return do_register_instance(key, instance_handle, wparams);
}

InstanceHandle_t DataWriterImpl::register_instance_w_timestamp(
        const void* const key,
        const fastdds::dds::Time_t& timestamp)
{
    /// Preconditions
    InstanceHandle_t instance_handle;
    if (timestamp.is_infinite() || timestamp.seconds < 0 ||
            (RETCODE_OK != check_instance_preconditions(key, HANDLE_NIL, instance_handle)))
    {
        return HANDLE_NIL;
    }

    WriteParams wparams;
    wparams.source_timestamp(timestamp);
    return do_register_instance(key, instance_handle, wparams);
}

InstanceHandle_t DataWriterImpl::do_register_instance(
        const void* const key,
        const InstanceHandle_t instance_handle,
        WriteParams& wparams)
{
    // TODO(MiguelCompany): wparams should be used when propagating the register_instance operation to the DataReader.
    // See redmine issue #14494
    static_cast<void>(wparams);

    // Block lowlevel writer
    auto max_blocking_time = std::chrono::steady_clock::now() +
            std::chrono::microseconds(rtps::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (lock.try_lock_until(max_blocking_time))
#else
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // if HAVE_STRICT_REALTIME
    {
        SerializedPayload_t* payload = nullptr;
        if (history_->register_instance(instance_handle, lock, max_blocking_time, payload))
        {
            // Keep serialization of sample inside the instance
            assert(nullptr != payload);
            if (0 == payload->length || nullptr == payload->data)
            {
                uint32_t size = fixed_payload_size_ ? fixed_payload_size_ : type_->calculate_serialized_size(key,
                                data_representation_);
                payload->reserve(size);
                if (!type_->serialize(key, *payload, data_representation_))
                {
                    EPROSIMA_LOG_WARNING(DATA_WRITER, "Key data serialization failed");

                    // Serialization of the sample failed. Remove the instance to keep original state.
                    // Note that we will only end-up here if the instance has just been created, so it will be empty
                    // and removing its changes will remove the instance completely.
                    history_->remove_instance_changes(instance_handle, rtps::SequenceNumber_t());
                }
            }
            return instance_handle;
        }
    }

    return HANDLE_NIL;
}

ReturnCode_t DataWriterImpl::unregister_instance(
        const void* const instance,
        const InstanceHandle_t& handle,
        bool dispose)
{
    // Preconditions
    InstanceHandle_t ih;
    ReturnCode_t returned_value = check_instance_preconditions(instance, handle, ih);
    if (RETCODE_OK == returned_value && !history_->is_key_registered(ih))
    {
        returned_value = RETCODE_PRECONDITION_NOT_MET;
    }

    // Operation
    if (RETCODE_OK == returned_value)
    {
        WriteParams wparams;
        ChangeKind_t change_kind = unregister_change_kind(dispose, qos_);
        returned_value = create_new_change_with_params(change_kind, instance, wparams, ih);
    }

    return returned_value;
}

ReturnCode_t DataWriterImpl::unregister_instance_w_timestamp(
        const void* const instance,
        const InstanceHandle_t& handle,
        const fastdds::dds::Time_t& timestamp,
        bool dispose)
{
    // Preconditions
    InstanceHandle_t instance_handle;
    ReturnCode_t ret = RETCODE_OK;
    if (timestamp.is_infinite() || timestamp.seconds < 0)
    {
        ret = RETCODE_BAD_PARAMETER;
    }
    if (RETCODE_OK == ret)
    {
        ret = check_instance_preconditions(instance, handle, instance_handle);
    }
    if (RETCODE_OK == ret && !history_->is_key_registered(instance_handle))
    {
        ret = RETCODE_PRECONDITION_NOT_MET;
    }

    // Operation
    if (RETCODE_OK == ret)
    {
        WriteParams wparams;
        wparams.source_timestamp(timestamp);
        ChangeKind_t change_kind = unregister_change_kind(dispose, qos_);
        ret = create_new_change_with_params(change_kind, instance, wparams, instance_handle);
    }

    return ret;
}

ReturnCode_t DataWriterImpl::get_key_value(
        void* key_holder,
        const InstanceHandle_t& handle)
{
    /// Preconditions
    if (key_holder == nullptr || !handle.isDefined())
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Key holder pointer not valid");
        return RETCODE_BAD_PARAMETER;
    }

    if (!type_->is_compute_key_provided)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Topic is NO_KEY, operation not permitted");
        return RETCODE_ILLEGAL_OPERATION;
    }

    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    // Block lowlevel writer
#if HAVE_STRICT_REALTIME
    auto max_blocking_time = std::chrono::steady_clock::now() +
            std::chrono::microseconds(rtps::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        return RETCODE_TIMEOUT;
    }
#else
    std::lock_guard<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // if HAVE_STRICT_REALTIME

    SerializedPayload_t* payload = history_->get_key_value(handle);
    if (nullptr == payload)
    {
        return RETCODE_BAD_PARAMETER;
    }

    type_->deserialize(*payload, key_holder);
    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::create_new_change(
        ChangeKind_t changeKind,
        const void* const data)
{
    WriteParams wparams;
    return create_new_change_with_params(changeKind, data, wparams);
}

ReturnCode_t DataWriterImpl::check_new_change_preconditions(
        ChangeKind_t change_kind,
        const void* const data)
{
    // Preconditions
    if (data == nullptr)
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Data pointer not valid");
        return RETCODE_BAD_PARAMETER;
    }

    if (change_kind == NOT_ALIVE_UNREGISTERED
            || change_kind == NOT_ALIVE_DISPOSED
            || change_kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        if (!type_->is_compute_key_provided)
        {
            EPROSIMA_LOG_ERROR(DATA_WRITER, "Topic is NO_KEY, operation not permitted");
            return RETCODE_ILLEGAL_OPERATION;
        }
    }

    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::perform_create_new_change(
        ChangeKind_t change_kind,
        const void* const data,
        WriteParams& wparams,
        const InstanceHandle_t& handle)
{
    // Block lowlevel writer
    auto max_blocking_time = steady_clock::now() +
            microseconds(rtps::TimeConv::Time_t2MicroSecondsInt64(qos_.reliability().max_blocking_time));

#if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        return RETCODE_TIMEOUT;
    }
#else
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // if HAVE_STRICT_REALTIME

    SerializedPayload_t payload;
    bool was_loaned = check_and_remove_loan(data, payload);
    if (!was_loaned)
    {
        uint32_t payload_size = fixed_payload_size_ ? fixed_payload_size_ : type_->calculate_serialized_size(
            data, data_representation_);
        if (!get_free_payload_from_pool(payload_size, payload))
        {
            return RETCODE_OUT_OF_RESOURCES;
        }

        if ((ALIVE == change_kind) && !type_->serialize(data, payload, data_representation_))
        {
            EPROSIMA_LOG_WARNING(DATA_WRITER, "Data serialization returned false");
            payload_pool_->release_payload(payload);
            return RETCODE_ERROR;
        }
    }

    CacheChange_t* ch = history_->create_change(change_kind, handle);
    if (ch != nullptr)
    {
        ch->serializedPayload = std::move(payload);

        bool added = false;
        if (reader_filters_)
        {
            auto related_sample_identity = wparams.related_sample_identity();
            auto filter_hook = [&related_sample_identity, this](CacheChange_t& ch)
                    {
                        reader_filters_->update_filter_info(static_cast<DataWriterFilteredChange&>(ch),
                                related_sample_identity);
                    };
            added = history_->add_pub_change_with_commit_hook(ch, wparams, filter_hook, lock, max_blocking_time);
        }
        else
        {
            added = history_->add_pub_change(ch, wparams, lock, max_blocking_time);
        }

        if (!added)
        {
            if (was_loaned)
            {
                payload = std::move(ch->serializedPayload);
                add_loan(data, payload);
            }
            history_->release_change(ch);
            return RETCODE_TIMEOUT;
        }

        if (qos_.deadline().period != dds::c_TimeInfinite)
        {
            if (!history_->set_next_deadline(
                        handle,
                        steady_clock::now() + duration_cast<steady_clock::duration>(deadline_duration_us_)))
            {
                EPROSIMA_LOG_ERROR(DATA_WRITER, "Could not set the next deadline in the history");
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

        if (qos_.lifespan().duration != dds::c_TimeInfinite)
        {
            lifespan_duration_us_ = duration<double, std::ratio<1, 1000000>>(
                qos_.lifespan().duration.to_ns() * 1e-3);
            lifespan_timer_->update_interval_millisec(qos_.lifespan().duration.to_ns() * 1e-6);
            lifespan_timer_->restart_timer();
        }

        return RETCODE_OK;
    }

    return RETCODE_OUT_OF_RESOURCES;
}

ReturnCode_t DataWriterImpl::create_new_change_with_params(
        ChangeKind_t changeKind,
        const void* const data,
        WriteParams& wparams)
{
    ReturnCode_t ret_code = check_new_change_preconditions(changeKind, data);
    if (RETCODE_OK != ret_code)
    {
        return ret_code;
    }

    InstanceHandle_t handle;
    if (type_->is_compute_key_provided)
    {
        bool is_key_protected = false;
#if HAVE_SECURITY
        is_key_protected = writer_->getAttributes().security_attributes().is_key_protected;
#endif // if HAVE_SECURITY
        type_->compute_key(data, handle, is_key_protected);
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}

ReturnCode_t DataWriterImpl::create_new_change_with_params(
        ChangeKind_t changeKind,
        const void* const data,
        WriteParams& wparams,
        const InstanceHandle_t& handle)
{
    ReturnCode_t ret_code = check_new_change_preconditions(changeKind, data);
    if (RETCODE_OK != ret_code)
    {
        return ret_code;
    }

    return perform_create_new_change(changeKind, data, wparams, handle);
}

bool DataWriterImpl::remove_min_seq_change()
{
    return history_->removeMinChange();
}

ReturnCode_t DataWriterImpl::clear_history(
        size_t* removed)
{
    return (history_->removeAllChange(removed) ? RETCODE_OK : RETCODE_ERROR);
}

ReturnCode_t DataWriterImpl::get_sending_locators(
        rtps::LocatorList& locators) const
{
    if (nullptr == writer_)
    {
        return RETCODE_NOT_ENABLED;
    }

    writer_->get_participant_impl()->get_sending_locators(locators);
    return RETCODE_OK;
}

const fastdds::rtps::GUID_t& DataWriterImpl::guid() const
{
    return guid_;
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
        publisher_->rtps_participant()->update_writer(writer_, wqos);
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
        ReturnCode_t ret_val = check_qos_including_resource_limits(qos_to_set, type_);
        if (RETCODE_OK != ret_val)
        {
            return ret_val;
        }

        if (publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data != 0 &&
                publisher_->get_participant()->get_qos().allocation().data_limits.max_user_data <
                qos_to_set.user_data().getValue().size())
        {
            return RETCODE_INCONSISTENT_POLICY;
        }
    }

    if (enabled && !can_qos_be_updated(qos_, qos_to_set))
    {
        return RETCODE_IMMUTABLE_POLICY;
    }

    set_qos(qos_, qos_to_set, !enabled);

    if (enabled)
    {
        if (qos_.reliability().kind == ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS &&
                qos_.reliable_writer_qos() == qos_to_set.reliable_writer_qos())
        {
            // Update times and positive_acks attributes on RTPS Layer
            WriterAttributes w_att;
            w_att.times = qos_.reliable_writer_qos().times;
            w_att.disable_positive_acks = qos_.reliable_writer_qos().disable_positive_acks.enabled;
            w_att.keep_duration = qos_.reliable_writer_qos().disable_positive_acks.duration;
            writer_->update_attributes(w_att);
        }

        //Notify the participant that a Writer has changed its QOS
        WriterQos wqos = qos_.get_writerqos(get_publisher()->get_qos(), topic_->get_qos());
        publisher_->rtps_participant()->update_writer(writer_, wqos);

        // Deadline
        if (qos_.deadline().period != dds::c_TimeInfinite)
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
        if (qos_.lifespan().duration != dds::c_TimeInfinite)
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

    return RETCODE_OK;
}

const DataWriterQos& DataWriterImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t DataWriterImpl::set_listener(
        DataWriterListener* listener)
{
    std::lock_guard<std::mutex> scoped_lock(listener_mutex_);
    listener_ = listener;
    return RETCODE_OK;
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

void DataWriterImpl::InnerDataWriterListener::on_writer_matched(
        RTPSWriter* /*writer*/,
        const MatchingInfo& info)
{
    data_writer_->update_publication_matched_status(info);

    StatusMask notify_status = StatusMask::publication_matched();
    DataWriterListener* listener = data_writer_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        PublicationMatchedStatus callback_status;
        if (RETCODE_OK == data_writer_->get_publication_matched_status(callback_status))
        {
            listener->on_publication_matched(data_writer_->user_datawriter_, callback_status);
        }
    }
    data_writer_->user_datawriter_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataWriterImpl::InnerDataWriterListener::on_offered_incompatible_qos(
        RTPSWriter* /*writer*/,
        fastdds::dds::PolicyMask qos)
{
    data_writer_->update_offered_incompatible_qos(qos);
    StatusMask notify_status = StatusMask::offered_incompatible_qos();
    DataWriterListener* listener = data_writer_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        OfferedIncompatibleQosStatus callback_status;
        if (data_writer_->get_offered_incompatible_qos_status(callback_status) == RETCODE_OK)
        {
            listener->on_offered_incompatible_qos(data_writer_->user_datawriter_, callback_status);
        }
    }

#ifdef FASTDDS_STATISTICS
    notify_status_observer(statistics::StatusKind::INCOMPATIBLE_QOS);
#endif //FASTDDS_STATISTICS

    data_writer_->user_datawriter_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataWriterImpl::InnerDataWriterListener::on_writer_change_received_by_all(
        RTPSWriter* /*writer*/,
        CacheChange_t* ch)
{
    if (data_writer_->type_->is_compute_key_provided &&
            (NOT_ALIVE_UNREGISTERED == ch->kind ||
            NOT_ALIVE_DISPOSED_UNREGISTERED == ch->kind))
    {
        data_writer_->history_->remove_instance_changes(ch->instanceHandle, ch->sequenceNumber);
    }
    else if (data_writer_->qos_.durability().kind == VOLATILE_DURABILITY_QOS)
    {
        data_writer_->history_->remove_change_g(ch);
    }
}

void DataWriterImpl::InnerDataWriterListener::on_liveliness_lost(
        fastdds::rtps::RTPSWriter* /*writer*/,
        const LivelinessLostStatus& status)
{
    data_writer_->update_liveliness_lost_status(status);
    StatusMask notify_status = StatusMask::liveliness_lost();
    DataWriterListener* listener = data_writer_->get_listener_for(notify_status);
    if (listener != nullptr)
    {
        LivelinessLostStatus callback_status;
        if (RETCODE_OK == data_writer_->get_liveliness_lost_status(callback_status))
        {
            listener->on_liveliness_lost(data_writer_->user_datawriter_, callback_status);
        }
    }

#ifdef FASTDDS_STATISTICS
    notify_status_observer(statistics::StatusKind::LIVELINESS_LOST);
#endif //FASTDDS_STATISTICS

    data_writer_->user_datawriter_->get_statuscondition().get_impl()->set_status(notify_status, true);
}

void DataWriterImpl::InnerDataWriterListener::on_reader_discovery(
        fastdds::rtps::RTPSWriter* writer,
        fastdds::rtps::ReaderDiscoveryStatus reason,
        const fastdds::rtps::GUID_t& reader_guid,
        const fastdds::rtps::SubscriptionBuiltinTopicData* reader_info)
{
    if (!fastdds::rtps::RTPSDomainImpl::should_intraprocess_between(writer->getGuid(), reader_guid))
    {
        switch (reason)
        {
            case fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER:
                data_writer_->remove_reader_filter(reader_guid);
                break;

            case fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER:
            case fastdds::rtps::ReaderDiscoveryStatus::CHANGED_QOS_READER:
                data_writer_->process_reader_filter_info(reader_guid, *reader_info);
                break;
            default:
                break;
        }
    }
}

#ifdef FASTDDS_STATISTICS
void DataWriterImpl::InnerDataWriterListener::notify_status_observer(
        const uint32_t& status_id)
{
    DomainParticipantImpl* pp_impl = data_writer_->publisher_->get_participant_impl();
    auto statistics_pp_impl = static_cast<eprosima::fastdds::statistics::dds::DomainParticipantImpl*>(pp_impl);
    if (nullptr != statistics_pp_impl->get_status_observer())
    {
        if (!statistics_pp_impl->get_status_observer()->on_local_entity_status_change(data_writer_->guid(), status_id))
        {
            EPROSIMA_LOG_ERROR(DATA_WRITER, "Could not set entity status");
        }
    }
}

#endif //FASTDDS_STATISTICS

ReturnCode_t DataWriterImpl::wait_for_acknowledgments(
        const dds::Duration_t& max_wait)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    if (writer_->wait_for_all_acked(max_wait))
    {
        return RETCODE_OK;
    }
    return RETCODE_ERROR;
}

ReturnCode_t DataWriterImpl::wait_for_acknowledgments(
        const void* const instance,
        const InstanceHandle_t& handle,
        const dds::Duration_t& max_wait)
{
    // Preconditions
    InstanceHandle_t ih;
    ReturnCode_t returned_value = check_instance_preconditions(instance, handle, ih);
    if (RETCODE_OK != returned_value)
    {
        return returned_value;
    }

    // Block low-level writer
    auto max_blocking_time = steady_clock::now() +
            microseconds(rtps::TimeConv::Time_t2MicroSecondsInt64(max_wait));

# if HAVE_STRICT_REALTIME
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex(), std::defer_lock);
    if (!lock.try_lock_until(max_blocking_time))
    {
        return RETCODE_TIMEOUT;
    }
#else
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());
#endif // HAVE_STRICT_REALTIME

    if (!history_->is_key_registered(ih))
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    if (history_->wait_for_acknowledgement_last_change(ih, lock, max_blocking_time))
    {
        return RETCODE_OK;
    }

    return RETCODE_TIMEOUT;
}

void DataWriterImpl::update_publication_matched_status(
        const MatchingInfo& status)
{
    auto count_change = status.status == MATCHED_MATCHING ? 1 : -1;
    publication_matched_status_.current_count += count_change;
    publication_matched_status_.current_count_change += count_change;
    if (count_change > 0)
    {
        publication_matched_status_.total_count += count_change;
        publication_matched_status_.total_count_change += count_change;
    }
    publication_matched_status_.last_subscription_handle = status.remoteEndpointGuid;
}

ReturnCode_t DataWriterImpl::get_publication_matched_status(
        PublicationMatchedStatus& status)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

        status = publication_matched_status_;
        publication_matched_status_.current_count_change = 0;
        publication_matched_status_.total_count_change = 0;
    }

    user_datawriter_->get_statuscondition().get_impl()->set_status(StatusMask::publication_matched(), false);
    return RETCODE_OK;
}

bool DataWriterImpl::deadline_timer_reschedule()
{
    assert(qos_.deadline().period != dds::c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    steady_clock::time_point next_deadline_us;
    if (!history_->get_next_deadline(timer_owner_, next_deadline_us))
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Could not get the next deadline from the history");
        return false;
    }

    auto interval_ms = duration_cast<milliseconds>(next_deadline_us - steady_clock::now());
    deadline_timer_->update_interval_millisec(static_cast<double>(interval_ms.count()));
    return true;
}

bool DataWriterImpl::deadline_missed()
{
    assert(qos_.deadline().period != dds::c_TimeInfinite);

    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    deadline_missed_status_.total_count++;
    deadline_missed_status_.total_count_change++;
    deadline_missed_status_.last_instance_handle = timer_owner_;
    StatusMask notify_status = StatusMask::offered_deadline_missed();
    auto listener = get_listener_for(notify_status);
    if (nullptr != listener)
    {
        listener->on_offered_deadline_missed(user_datawriter_, deadline_missed_status_);
        deadline_missed_status_.total_count_change = 0;
    }

#ifdef FASTDDS_STATISTICS
    writer_listener_.notify_status_observer(statistics::StatusKind::DEADLINE_MISSED);
#endif //FASTDDS_STATISTICS

    user_datawriter_->get_statuscondition().get_impl()->set_status(notify_status, true);

    if (!history_->set_next_deadline(
                timer_owner_,
                steady_clock::now() + duration_cast<steady_clock::duration>(deadline_duration_us_)))
    {
        EPROSIMA_LOG_ERROR(DATA_WRITER, "Could not set the next deadline in the history");
        return false;
    }
    return deadline_timer_reschedule();
}

ReturnCode_t DataWriterImpl::get_offered_deadline_missed_status(
        OfferedDeadlineMissedStatus& status)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

        status = deadline_missed_status_;
        deadline_missed_status_.total_count_change = 0;
    }

    user_datawriter_->get_statuscondition().get_impl()->set_status(StatusMask::offered_deadline_missed(), false);
    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::get_offered_incompatible_qos_status(
        OfferedIncompatibleQosStatus& status)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

        status = offered_incompatible_qos_status_;
        offered_incompatible_qos_status_.total_count_change = 0u;
    }

    user_datawriter_->get_statuscondition().get_impl()->set_status(StatusMask::offered_incompatible_qos(), false);
    return RETCODE_OK;
}

bool DataWriterImpl::lifespan_expired()
{
    std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

    fastdds::rtps::Time_t current_ts;
    fastdds::rtps::Time_t::now(current_ts);

    CacheChange_t* earliest_change;
    while (history_->get_earliest_change(&earliest_change))
    {
        fastdds::rtps::Time_t expiration_ts = earliest_change->sourceTimestamp + qos_.lifespan().duration;

        // Check that the earliest change has expired (the change which started the timer could have been removed from the history)
        if (current_ts < expiration_ts)
        {
            fastdds::rtps::Time_t interval = expiration_ts - current_ts;
            lifespan_timer_->update_interval_millisec(interval.to_ns() * 1e-6);
            return true;
        }

        // The earliest change has expired
        history_->remove_change_pub(earliest_change);
    }

    return false;
}

ReturnCode_t DataWriterImpl::get_liveliness_lost_status(
        LivelinessLostStatus& status)
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    {
        std::unique_lock<RecursiveTimedMutex> lock(writer_->getMutex());

        status = liveliness_lost_status_;
        liveliness_lost_status_.total_count_change = 0u;
    }

    user_datawriter_->get_statuscondition().get_impl()->set_status(StatusMask::liveliness_lost(), false);
    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::assert_liveliness()
{
    if (writer_ == nullptr)
    {
        return RETCODE_NOT_ENABLED;
    }

    if (!publisher_->rtps_participant()->wlp()->assert_liveliness(
                writer_->getGuid(),
                writer_->get_liveliness_kind(),
                writer_->get_liveliness_lease_duration()))
    {
        EPROSIMA_LOG_ERROR(DATAWRITER, "Could not assert liveliness of writer " << writer_->getGuid());
        return RETCODE_ERROR;
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
    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::get_publication_builtin_topic_data(
        PublicationBuiltinTopicData& publication_data) const
{
    if (nullptr == writer_)
    {
        return RETCODE_NOT_ENABLED;
    }

    // sanity checks
    assert(nullptr != publisher_);
    assert(nullptr != topic_);
    assert(nullptr != publisher_->get_participant());
    assert(nullptr != writer_->get_participant_impl());

    publication_data = PublicationBuiltinTopicData{};

    from_entity_id_to_topic_key(guid_.entityId, publication_data.key.value);
    from_guid_prefix_to_topic_key(
        publisher_->get_participant()->guid().guidPrefix, publication_data.participant_key.value);

    publication_data.topic_name = topic_->get_name();
    publication_data.type_name = topic_->get_type_name();
    publication_data.topic_kind = type_->is_compute_key_provided ? TopicKind_t::WITH_KEY : TopicKind_t::NO_KEY;

    // DataWriter qos
    publication_data.durability = qos_.durability();
    publication_data.durability_service = qos_.durability_service();
    publication_data.deadline = qos_.deadline();
    publication_data.latency_budget = qos_.latency_budget();
    publication_data.liveliness = qos_.liveliness();
    publication_data.reliability = qos_.reliability();
    publication_data.lifespan = qos_.lifespan();
    publication_data.user_data = qos_.user_data();
    publication_data.ownership = qos_.ownership();
    publication_data.ownership_strength = qos_.ownership_strength();
    publication_data.destination_order = qos_.destination_order();

    // Publisher qos
    publication_data.presentation = publisher_->qos_.presentation();
    publication_data.partition = publisher_->qos_.partition();
    publication_data.topic_data = topic_->get_qos().topic_data();
    publication_data.group_data = publisher_->qos_.group_data();

    // XTypes 1.3
    publisher_->get_participant_impl()->fill_type_information(type_, publication_data.type_information);
    publication_data.representation = qos_.representation();

    // eProsima extensions

    publication_data.disable_positive_acks = qos_.reliable_writer_qos().disable_positive_acks;
    publication_data.data_sharing = qos_.data_sharing();

    if (publication_data.data_sharing.kind() != OFF &&
            publication_data.data_sharing.domain_ids().empty())
    {
        publication_data.data_sharing.add_domain_id(utils::default_domain_id());
    }

    publication_data.guid = guid();
    publication_data.participant_guid = publisher_->get_participant()->guid();

    const std::string* pers_guid = PropertyPolicyHelper::find_property(qos_.properties(), "dds.persistence.guid");
    if (pers_guid)
    {
        // Load persistence_guid from property
        std::istringstream(pers_guid->c_str()) >> publication_data.persistence_guid;
    }

    qos_.endpoint().unicast_locator_list.copy_to(publication_data.remote_locators.unicast);
    qos_.endpoint().multicast_locator_list.copy_to(publication_data.remote_locators.multicast);
    publication_data.max_serialized_size = type_->max_serialized_type_size;
    publication_data.loopback_transformation =
            writer_->get_participant_impl()->network_factory().network_configuration();

    if (!is_data_sharing_compatible_)
    {
        publication_data.data_sharing.off();
    }

    const std::string* endpoint_partitions = PropertyPolicyHelper::find_property(qos_.properties(), "partitions");
    if (endpoint_partitions)
    {
        std::istringstream partition_string(*endpoint_partitions);
        std::string partition_name;
        publication_data.partition.clear();

        while (std::getline(partition_string, partition_name, ';'))
        {
            publication_data.partition.push_back(partition_name.c_str());
        }
    }

    return RETCODE_OK;
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

LivelinessLostStatus& DataWriterImpl::update_liveliness_lost_status(
        const LivelinessLostStatus& liveliness_lost_status)
{
    liveliness_lost_status_.total_count = liveliness_lost_status.total_count;
    liveliness_lost_status_.total_count_change += liveliness_lost_status.total_count_change;
    return liveliness_lost_status_;
}

void DataWriterImpl::set_qos(
        DataWriterQos& to,
        const DataWriterQos& from,
        bool update_immutable)
{
    // Check immutable policies
    if (update_immutable)
    {
        if (!(to.durability() == from.durability()))
        {
            to.durability() = from.durability();
            to.durability().hasChanged = true;
        }

        if (!(to.durability_service() == from.durability_service()))
        {
            to.durability_service() = from.durability_service();
            to.durability_service().hasChanged = true;
        }

        if (!(to.liveliness() == from.liveliness()))
        {
            to.liveliness() = from.liveliness();
            to.liveliness().hasChanged = true;
        }

        if (!(to.reliability().kind == from.reliability().kind))
        {
            to.reliability().kind = from.reliability().kind;
            to.reliability().hasChanged = true;
        }

        if (!(to.destination_order() == from.destination_order()))
        {
            to.destination_order() = from.destination_order();
            to.destination_order().hasChanged = true;
        }

        if (!(to.history() == from.history()))
        {
            to.history() = from.history();
            to.history().hasChanged = true;
        }

        if (!(to.resource_limits() == from.resource_limits()))
        {
            to.resource_limits() = from.resource_limits();
            to.resource_limits().hasChanged = true;
        }

        if (!(to.ownership() == from.ownership()))
        {
            to.ownership() = from.ownership();
            to.ownership().hasChanged = true;
        }

        to.publish_mode() = from.publish_mode();

        if (!(to.representation() == from.representation()))
        {
            to.representation() = from.representation();
            to.representation().hasChanged = true;
        }

        to.properties() = from.properties();

        if (!(to.reliable_writer_qos() == from.reliable_writer_qos()))
        {
            RTPSReliableWriterQos& rel_to = to.reliable_writer_qos();
            rel_to.disable_heartbeat_piggyback = from.reliable_writer_qos().disable_heartbeat_piggyback;
            rel_to.disable_positive_acks.enabled = from.reliable_writer_qos().disable_positive_acks.enabled;
        }

        to.endpoint() = from.endpoint();

        to.writer_resource_limits() = from.writer_resource_limits();

        to.data_sharing() = from.data_sharing();
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

    if (!(to.reliability().max_blocking_time == from.reliability().max_blocking_time))
    {
        to.reliability().max_blocking_time = from.reliability().max_blocking_time;
        to.reliability().hasChanged = true;
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

    if (!(to.ownership_strength() == from.ownership_strength()))
    {
        to.ownership_strength() = from.ownership_strength();
        to.ownership_strength().hasChanged = true;
    }

    if (!(to.writer_data_lifecycle() == from.writer_data_lifecycle()))
    {
        to.writer_data_lifecycle() = from.writer_data_lifecycle();
    }

    if (!(to.reliable_writer_qos() == from.reliable_writer_qos()))
    {
        RTPSReliableWriterQos& rel_to = to.reliable_writer_qos();
        rel_to.times = from.reliable_writer_qos().times;
        rel_to.disable_positive_acks.duration = from.reliable_writer_qos().disable_positive_acks.duration;
    }
}

ReturnCode_t DataWriterImpl::check_qos_including_resource_limits(
        const DataWriterQos& qos,
        const TypeSupport& type)
{
    ReturnCode_t check_qos_return = check_qos(qos);
    if (RETCODE_OK == check_qos_return &&
            type->is_compute_key_provided)
    {
        check_qos_return = check_allocation_consistency(qos);
    }
    return check_qos_return;
}

ReturnCode_t DataWriterImpl::check_qos(
        const DataWriterQos& qos)
{
    if (qos.destination_order().kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return RETCODE_UNSUPPORTED;
    }
    if (nullptr != PropertyPolicyHelper::find_property(qos.properties(), "fastdds.unique_network_flows"))
    {
        EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "Unique network flows not supported on writers");
        return RETCODE_UNSUPPORTED;
    }
    bool is_pull_mode = qos_has_pull_mode_request(qos);
    if (is_pull_mode)
    {
        if (BEST_EFFORT_RELIABILITY_QOS == qos.reliability().kind)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "BEST_EFFORT incompatible with pull mode");
            return RETCODE_INCONSISTENT_POLICY;
        }
        if (dds::c_TimeInfinite == qos.reliable_writer_qos().times.heartbeat_period)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "Infinite heartbeat period incompatible with pull mode");
            return RETCODE_INCONSISTENT_POLICY;
        }
    }
    if (qos.liveliness().kind == AUTOMATIC_LIVELINESS_QOS ||
            qos.liveliness().kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (qos.liveliness().lease_duration < eprosima::fastdds::dds::c_TimeInfinite &&
                qos.liveliness().lease_duration <= qos.liveliness().announcement_period)
        {
            EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "WRITERQOS: LeaseDuration <= announcement period.");
            return RETCODE_INCONSISTENT_POLICY;
        }
    }
    if (qos.data_sharing().kind() == DataSharingKind::ON &&
            (qos.endpoint().history_memory_policy != PREALLOCATED_MEMORY_MODE &&
            qos.endpoint().history_memory_policy != PREALLOCATED_WITH_REALLOC_MEMORY_MODE))
    {
        EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "DATA_SHARING cannot be used with memory policies other than PREALLOCATED.");
        return RETCODE_INCONSISTENT_POLICY;
    }
    if (qos.history().kind == KEEP_LAST_HISTORY_QOS && qos.history().depth <= 0)
    {
        EPROSIMA_LOG_ERROR(RTPS_QOS_CHECK, "HISTORY DEPTH must be higher than 0 if HISTORY KIND is KEEP_LAST.");
        return RETCODE_INCONSISTENT_POLICY;
    }
    if (qos.history().kind == KEEP_LAST_HISTORY_QOS && qos.history().depth > 0 &&
            qos.resource_limits().max_samples_per_instance > 0 &&
            qos.history().depth > qos.resource_limits().max_samples_per_instance)
    {
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "HISTORY DEPTH '" << qos.history().depth <<
                "' is inconsistent with max_samples_per_instance: '" << qos.resource_limits().max_samples_per_instance <<
                "'. Consistency rule: depth <= max_samples_per_instance." <<
                " Effectively using max_samples_per_instance as depth.");
    }
    return RETCODE_OK;
}

ReturnCode_t DataWriterImpl::check_allocation_consistency(
        const DataWriterQos& qos)
{
    if ((qos.resource_limits().max_samples > 0) &&
            (qos.resource_limits().max_samples <
            (qos.resource_limits().max_instances * qos.resource_limits().max_samples_per_instance)))
    {
        EPROSIMA_LOG_ERROR(DDS_QOS_CHECK,
                "max_samples should be greater than max_instances * max_samples_per_instance");
        return RETCODE_INCONSISTENT_POLICY;
    }
    if ((qos.resource_limits().max_instances <= 0 || qos.resource_limits().max_samples_per_instance <= 0) &&
            (qos.resource_limits().max_samples > 0))
    {
        EPROSIMA_LOG_ERROR(DDS_QOS_CHECK,
                "max_samples should be infinite when max_instances or max_samples_per_instance are infinite");
        return RETCODE_INCONSISTENT_POLICY;
    }
    return RETCODE_OK;
}

bool DataWriterImpl::can_qos_be_updated(
        const DataWriterQos& to,
        const DataWriterQos& from)
{
    bool updatable = true;
    if (to.durability().kind != from.durability().kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().kind !=  from.liveliness().kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().lease_duration != from.liveliness().lease_duration)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness lease duration cannot be changed after the creation of a DataWriter.");
    }

    if (to.liveliness().announcement_period != from.liveliness().announcement_period)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness announcement cannot be changed after the creation of a DataWriter.");
    }

    if (to.reliability().kind != from.reliability().kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a DataWriter.");
    }
    if (to.ownership().kind != from.ownership().kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a DataWriter.");
    }
    if (to.destination_order().kind != from.destination_order().kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Destination order Kind cannot be changed after the creation of a DataWriter.");
    }
    if (to.data_sharing().kind() != from.data_sharing().kind())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a DataWriter.");
    }
    if (to.data_sharing().shm_directory() != from.data_sharing().shm_directory())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a DataWriter.");
    }
    if (to.data_sharing().domain_ids() != from.data_sharing().domain_ids())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a DataWriter.");
    }
    if (to.reliable_writer_qos().disable_positive_acks.enabled !=
            from.reliable_writer_qos().disable_positive_acks.enabled)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Only the period of Positive ACKs can be changed after the creation of a DataWriter.");
    }
    if (to.properties() != from.properties())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "PropertyPolicyQos cannot be changed after the DataWriter is enabled.");
    }

    return updatable;
}

DataWriterListener* DataWriterImpl::get_listener_for(
        const StatusMask& status)
{
    std::lock_guard<std::mutex> scoped_lock(listener_mutex_);
    if (listener_ != nullptr &&
            user_datawriter_->get_status_mask().is_active(status))
    {
        return listener_;
    }
    return publisher_->get_listener_for(status);
}

std::shared_ptr<IChangePool> DataWriterImpl::get_change_pool() const
{
    if (reader_filters_)
    {
        return std::make_shared<DataWriterFilteredChangePool>(
            pool_config_, qos_.writer_resource_limits().reader_filters_allocation);
    }

    return std::make_shared<fastdds::rtps::CacheChangePool>(pool_config_);
}

std::shared_ptr<IPayloadPool> DataWriterImpl::get_payload_pool()
{
    if (!payload_pool_)
    {
        // Avoid calling the serialization size functors on PREALLOCATED mode
        fixed_payload_size_ =
                pool_config_.memory_policy == PREALLOCATED_MEMORY_MODE ? pool_config_.payload_initial_size : 0u;

        // Get payload pool reference and allocate space for our history
        if (is_data_sharing_compatible_)
        {
            payload_pool_ = DataSharingPayloadPool::get_writer_pool(pool_config_);
        }
        else
        {
            payload_pool_ = TopicPayloadPoolRegistry::get(topic_->get_name(), pool_config_);
            if (!std::static_pointer_cast<ITopicPayloadPool>(payload_pool_)->reserve_history(pool_config_, false))
            {
                payload_pool_.reset();
            }
        }

        // Prepare loans collection for plain types only
        if (type_->is_plain(data_representation_))
        {
            loans_.reset(new LoanCollection(pool_config_));
        }
    }

    return payload_pool_;
}

bool DataWriterImpl::release_payload_pool()
{
    assert(payload_pool_);

    loans_.reset();

    bool result = true;

    if (is_data_sharing_compatible_ || is_custom_payload_pool_)
    {
        // No-op
    }
    else
    {
        auto topic_pool = std::static_pointer_cast<ITopicPayloadPool>(payload_pool_);
        result = topic_pool->release_history(pool_config_, false);
    }

    payload_pool_.reset();

    return result;
}

bool DataWriterImpl::get_free_payload_from_pool(
        uint32_t size,
        SerializedPayload_t& payload)
{
    if (!payload_pool_)
    {
        return false;
    }

    if (!payload_pool_->get_payload(size, payload))
    {
        return false;
    }

    return true;
}

bool DataWriterImpl::add_loan(
        const void* const data,
        SerializedPayload_t& payload)
{
    return loans_ && loans_->add_loan(data, payload);
}

bool DataWriterImpl::check_and_remove_loan(
        const void* const data,
        SerializedPayload_t& payload)
{
    return loans_ && loans_->check_and_remove_loan(data, payload);
}

ReturnCode_t DataWriterImpl::check_datasharing_compatible(
        const WriterAttributes& writer_attributes,
        bool& is_datasharing_compatible) const
{

#if HAVE_SECURITY
    bool has_security_enabled = publisher_->rtps_participant()->is_security_enabled_for_writer(writer_attributes);
#else
    (void) writer_attributes;
#endif // HAVE_SECURITY

    bool has_bound_payload_size =
            (qos_.endpoint().history_memory_policy == eprosima::fastdds::rtps::PREALLOCATED_MEMORY_MODE ||
            qos_.endpoint().history_memory_policy == eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE) &&
            type_.is_bounded();

    bool has_key = type_->is_compute_key_provided;

    is_datasharing_compatible = false;
    switch (qos_.data_sharing().kind())
    {
        case DataSharingKind::OFF:
            return RETCODE_OK;
            break;
        case DataSharingKind::ON:
            if (is_custom_payload_pool_)
            {
                EPROSIMA_LOG_ERROR(DATA_WRITER, "Custom payload pool detected. Cannot force Data sharing usage.");
                return RETCODE_INCONSISTENT_POLICY;
            }
#if HAVE_SECURITY
            if (has_security_enabled)
            {
                EPROSIMA_LOG_ERROR(DATA_WRITER, "Data sharing cannot be used with security protection.");
                return RETCODE_NOT_ALLOWED_BY_SECURITY;
            }
#endif // HAVE_SECURITY

            if (!has_bound_payload_size)
            {
                EPROSIMA_LOG_ERROR(DATA_WRITER, "Data sharing cannot be used with " <<
                        (type_.is_bounded() ? "memory policies other than PREALLOCATED" : "unbounded data types"));
                return RETCODE_BAD_PARAMETER;
            }

            if (has_key)
            {
                EPROSIMA_LOG_ERROR(DATA_WRITER, "Data sharing cannot be used with keyed data types");
                return RETCODE_BAD_PARAMETER;
            }

            is_datasharing_compatible = true;
            return RETCODE_OK;
            break;
        case DataSharingKind::AUTO:
            if (is_custom_payload_pool_)
            {
                EPROSIMA_LOG_INFO(DATA_WRITER, "Custom payload pool detected. Data Sharing disabled.");
                return RETCODE_OK;
            }
#if HAVE_SECURITY
            if (has_security_enabled)
            {
                EPROSIMA_LOG_INFO(DATA_WRITER, "Data sharing disabled due to security configuration.");
                return RETCODE_OK;
            }
#endif // HAVE_SECURITY

            if (!has_bound_payload_size)
            {
                EPROSIMA_LOG_INFO(DATA_WRITER, "Data sharing disabled because " <<
                        (type_.is_bounded() ? "memory policy is not PREALLOCATED" : "data type is not bounded"));
                return RETCODE_OK;
            }

            if (has_key)
            {
                EPROSIMA_LOG_INFO(DATA_WRITER, "Data sharing disabled because data type is keyed");
                return RETCODE_OK;
            }

            is_datasharing_compatible = true;
            return RETCODE_OK;
            break;
        default:
            EPROSIMA_LOG_ERROR(DATA_WRITER, "Unknown data sharing kind.");
            return RETCODE_BAD_PARAMETER;
    }
}

void DataWriterImpl::remove_reader_filter(
        const fastdds::rtps::GUID_t& reader_guid)
{
    if (reader_filters_)
    {
        assert(writer_);
        std::lock_guard<RecursiveTimedMutex> guard(writer_->getMutex());
        reader_filters_->remove_reader(reader_guid);
    }
}

void DataWriterImpl::process_reader_filter_info(
        const fastdds::rtps::GUID_t& reader_guid,
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_info)
{
    if (reader_filters_ &&
            !writer_->is_datasharing_compatible_with(reader_info.data_sharing) &&
            reader_info.remote_locators.multicast.empty())
    {
        reader_filters_->process_reader_filter_info(reader_guid, reader_info.content_filter,
                publisher_->get_participant_impl(), topic_);
    }
}

void DataWriterImpl::filter_is_being_removed(
        const char* filter_class_name)
{
    if (reader_filters_)
    {
        assert(writer_);
        std::lock_guard<RecursiveTimedMutex> guard(writer_->getMutex());
        reader_filters_->remove_filters(filter_class_name);
    }
}

ReturnCode_t DataWriterImpl::get_matched_subscription_data(
        SubscriptionBuiltinTopicData& subscription_data,
        const InstanceHandle_t& subscription_handle) const
{
    ReturnCode_t ret = RETCODE_BAD_PARAMETER;
    fastdds::rtps::GUID_t reader_guid = iHandle2GUID(subscription_handle);

    if (writer_ && writer_->matched_reader_is_matched(reader_guid))
    {
        if (publisher_)
        {
            RTPSParticipant* rtps_participant = publisher_->rtps_participant();
            if (rtps_participant &&
                    rtps_participant->get_subscription_info(subscription_data, reader_guid))
            {
                ret = RETCODE_OK;
            }
        }
    }

    return ret;
}

ReturnCode_t DataWriterImpl::get_matched_subscriptions(
        std::vector<InstanceHandle_t>& subscription_handles) const
{
    ReturnCode_t ret = RETCODE_ERROR;
    std::vector<rtps::GUID_t> matched_reader_guids;
    subscription_handles.clear();

    if (writer_ && writer_->matched_readers_guids(matched_reader_guids))
    {
        for (const rtps::GUID_t& guid : matched_reader_guids)
        {
            subscription_handles.emplace_back(InstanceHandle_t(guid));
        }
        ret = RETCODE_OK;
    }

    return ret;
}

bool DataWriterImpl::is_relevant(
        const fastdds::rtps::CacheChange_t& change,
        const fastdds::rtps::GUID_t& reader_guid) const
{
    assert(reader_filters_);
    const DataWriterFilteredChange& writer_change = static_cast<const DataWriterFilteredChange&>(change);
    return writer_change.is_relevant_for(reader_guid);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
