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

/**
 * @file DataReader.hpp
 *
 */

#ifndef _FASTRTPS_DATAREADER_HPP_
#define _FASTRTPS_DATAREADER_HPP_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/core/Entity.hpp>
#include <fastrtps/types/TypesBase.h>

#include <vector>
#include <cstdint>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace sub {

class DataReader;

} // namespace sub
} // namespace dds

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

namespace rtps {
class ReaderAttributes;
struct GUID_t;
struct InstanceHandle_t;
} // namespace rtps

class SampleInfo_t;

} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class TypeSupport;
class DataReaderQos;
class TopicDescription;
struct LivelinessChangedStatus;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class DataReader : public DomainEntity
{
    friend class DataReaderImpl;
    friend class SubscriberImpl;

    /**
     * Creates a DataReader. Don't use it directly, but through Subscriber.
     */
    RTPS_DllAPI DataReader(
            DataReaderImpl* impl,
            const StatusMask& mask = StatusMask::all());

    RTPS_DllAPI DataReader(
            Subscriber* s,
            TopicDescription* topic,
            const DataReaderQos& qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all());

public:

    RTPS_DllAPI virtual ~DataReader();

    /**
     * Method to block the current thread until an unread message is available
     */
    RTPS_DllAPI bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    /* TODO
       RTPS_DllAPI bool read(
            std::vector<void*>& data_values,
            std::vector<fastrtps::SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    RTPS_DllAPI ReturnCode_t read_next_sample(
            void* data,
            fastrtps::SampleInfo_t* info);

    /* TODO
       RTPS_DllAPI bool take(
            std::vector<void*>& data_values,
            std::vector<fastrtps::SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    RTPS_DllAPI ReturnCode_t take_next_sample(
            void* data,
            fastrtps::SampleInfo_t* info);

    ///@}

    /**
     * @brief Returns information about the first untaken sample.
     * @param [out] info Pointer to a SampleInfo_t structure to store first untaken sample information.
     * @return RETCODE_OK if sample info was returned. RETCODE_NO_DATA if there is no sample to take.
     */
    RTPS_DllAPI ReturnCode_t get_first_untaken_info(
            fastrtps::SampleInfo_t* info);

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    RTPS_DllAPI const fastrtps::rtps::GUID_t& guid();

    RTPS_DllAPI fastrtps::rtps::InstanceHandle_t get_instance_handle() const;

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport type();

    /**
     * Get TopicDescription
     * @return TopicDescription
     */
    const TopicDescription* get_topicdescription() const;

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    RTPS_DllAPI ReturnCode_t get_requested_deadline_missed_status(
            fastrtps::RequestedDeadlineMissedStatus& status);

    RTPS_DllAPI ReturnCode_t set_qos(
            const DataReaderQos& qos);

    RTPS_DllAPI const DataReaderQos& get_qos() const;

    RTPS_DllAPI ReturnCode_t get_qos(
            DataReaderQos& qos) const;

    RTPS_DllAPI ReturnCode_t set_listener(
            DataReaderListener* listener);

    RTPS_DllAPI const DataReaderListener* get_listener() const;

    /* TODO
       RTPS_DllAPI bool get_key_value(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    RTPS_DllAPI ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status) const;

    /* TODO
       RTPS_DllAPI bool get_requested_incompatible_qos_status(
            fastrtps::RequestedIncompatibleQosStatus& status) const;
     */

    /* TODO
       RTPS_DllAPI bool get_sample_lost_status(
            fastrtps::SampleLostStatus& status) const;
     */

    /* TODO
       RTPS_DllAPI bool get_sample_rejected_status(
            fastrtps::SampleRejectedStatus& status) const;
     */

    RTPS_DllAPI const Subscriber* get_subscriber() const;

    /* TODO
       RTPS_DllAPI bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
     */

private:

    DataReaderImpl* impl_;

    friend class ::dds::sub::DataReader;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTRTPS_DATAREADER_HPP_*/
