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
#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/dds/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastrtps/types/TypesBase.h>

#include <dds/core/status/Status.hpp>
#include <dds/core/status/State.hpp>
#include <dds/sub/status/DataState.hpp>

#include <vector>
#include <cstdint>

using eprosima::fastrtps::types::ReturnCode_t;

namespace dds {
namespace sub {
template<typename T>
class DataReader;
class SampleInfo;
} // sub
} // dds

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

namespace rtps {
class ReaderAttributes;
struct GUID_t;
struct InstanceHandle_t;
} // namespace rtps

} // namespace fastrtps

namespace fastdds {
namespace dds {

class Subscriber;
class SubscriberImpl;
class DataReaderImpl;
class DataReaderListener;
class TypeSupport;
class Topic;
class TopicDescription;
struct LivelinessChangedStatus;
class SampleInfo_t;
class ReadCondition;

/**
 * Class DataReader, contains the actual implementation of the behaviour of the Subscriber.
 *  @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI DataReader : public DomainEntity
{
    friend class SubscriberImpl;

    template<typename T>
    friend class ::dds::sub::DataReader;

    DataReader(
            const Subscriber* pub,
            Topic* topic,
            const DataReaderQos& qos = DDS_DATAREADER_QOS_DEFAULT,
            DataReaderListener* listener = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    DataReader(
            const Subscriber* pub,
            const TopicDescription& topic_desc,
            const DataReaderQos& qos = DDS_DATAREADER_QOS_DEFAULT,
            DataReaderListener* listener = nullptr,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    /**
     * Creates a DataReader. Don't use it directly, but through Subscriber.
     */
    DataReader(
            DataReaderImpl* impl,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

public:

    virtual ~DataReader();

    /**
     * Method to block the current thread until an unread message is available
     */
    bool wait_for_unread_message(
            const fastrtps::Duration_t& timeout);


    /** @name Read or take data methods.
     * Methods to read or take data from the History.
     */

    ///@{

    /* TODO
       bool read(
            std::vector<void*>& data_values,
            std::vector<SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    ReturnCode_t read_next_sample(
            void* data,
            SampleInfo_t* info);

    /* TODO
       bool take(
            std::vector<void*>& data_values,
            std::vector<SampleInfo_t>& sample_infos,
            uint32_t max_samples);
     */

    ReturnCode_t take_next_sample(
            void* data,
            SampleInfo_t* info);

    ReturnCode_t take_next_sample(
            void* data,
            ::dds::sub::SampleInfo& info);

    ///@}

    /**
     * Get associated GUID
     * @return Associated GUID
     */
    const fastrtps::rtps::GUID_t& guid();

    /**
     * Get topic data type
     * @return Topic data type
     */
    TypeSupport type();

    /**
     * @brief Get the requested deadline missed status
     * @return The deadline missed status
     */
    ReturnCode_t get_requested_deadline_missed_status(
            fastrtps::RequestedDeadlineMissedStatus& status);

    bool set_attributes(
            const fastrtps::rtps::ReaderAttributes& att);

    const fastrtps::rtps::ReaderAttributes& get_attributes() const;

    ReturnCode_t set_qos(
            const DataReaderQos& qos);

    const DataReaderQos& get_qos() const;

    ReturnCode_t get_qos(
            DataReaderQos& qos) const;

    bool set_topic(
            const fastrtps::TopicAttributes& att);

    Topic* get_topic();

    const fastrtps::TopicAttributes& get_topic_attributes() const;

    TopicDescription* get_topicdescription() const;

    ReturnCode_t set_listener(
            DataReaderListener* listener,
            const ::dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all());

    const DataReaderListener* get_listener() const;

    ReadCondition* create_readcondition(
            ::dds::sub::status::SampleState sample_states,
            ::dds::sub::status::ViewState view_states,
            ::dds::sub::status::InstanceState instance_states);

    ReturnCode_t delete_readcondition(
            ReadCondition* condition);

    ReturnCode_t delete_contained_entities();

    /* TODO
       bool get_key_value(
            void* data,
            const fastrtps::rtps::InstanceHandle_t& handle);
     */

    ReturnCode_t get_liveliness_changed_status(
            LivelinessChangedStatus& status) const;

    ReturnCode_t get_requested_incompatible_qos_status(
            RequestedIncompatibleQosStatus& status) const;

    ReturnCode_t get_sample_lost_status(
            SampleLostStatus& status) const;

    ReturnCode_t get_sample_rejected_status(
            SampleRejectedStatus& status) const;

    ReturnCode_t get_subscription_matched_status(
            SubscriptionMatchedStatus& status);

    const Subscriber* get_subscriber() const;

    /* TODO
       bool wait_for_historical_data(
            const fastrtps::Duration_t& max_wait) const;
     */

    ReturnCode_t get_matched_publication_data(
            PublicationBuiltinTopicData& publication_data,
            const fastrtps::rtps::InstanceHandle_t& publication_handle) const;

    ReturnCode_t get_matched_publications(
            std::vector<fastrtps::rtps::InstanceHandle_t>& publication_handles) const;

    ReturnCode_t enable();

private:

    DataReaderImpl* impl_;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif
#endif /* _FASTRTPS_DATAREADER_HPP_*/
