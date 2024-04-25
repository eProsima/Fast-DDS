// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeIntrospectionSubscriber.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONSUBSCRIBER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONSUBSCRIBER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>


#include "types/types.hpp"

/**
 * Class used to group into a single working unit a Subscriber with a DataReader, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class TypeIntrospectionSubscriber : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    TypeIntrospectionSubscriber(
            const std::string& topic_name,
            uint32_t domain);

    virtual ~TypeIntrospectionSubscriber();

    //! RUN the subscriber until number samples are received
    void run(
            uint32_t number);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    //! Callback executed when a DomainParticipant is discovered, removed or changed QoS
    void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info,
            bool& /*should_be_ignored*/) override;

    //! Callback executed when a DataWriter is discovered, removed, ignored or changed QoS
    void on_data_writer_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastrtps::rtps::WriterDiscoveryInfo&& info,
            bool& /*should_be_ignored*/) override;

    //! Callback executed when a DataWriter is matched or unmatched
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    //! Callback executed when a new sample is received
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

protected:

    void on_type_discovered_and_registered_(
            const eprosima::fastdds::dds::DynamicType::_ref_type& type);

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    std::string topic_name_;

    std::string type_name_;

    //! Number of DataWriters matched to the associated DataReader
    int matched_;

    //! Number of samples received
    uint32_t samples_;

    //! Number of messages to be received before triggering termination of execution
    uint32_t max_messages_;

    //! Whether the type has been discovered
    static std::atomic<bool> type_discovered_;
    static std::atomic<bool> type_registered_;

    //! Protects type_discovered condition variable
    static std::mutex type_discovered_cv_mtx_;

    //! Waits until a type has been discovered
    static std::condition_variable type_discovered_cv_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;

    // Dynamic data information
    eprosima::fastdds::dds::DynamicType::_ref_type dyn_type_;

    // Instances count received
    std::set<eprosima::fastdds::dds::InstanceHandle_t> instances_;
};

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONSUBSCRIBER_H_ */
