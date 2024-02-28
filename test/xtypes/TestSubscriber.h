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
 * @file TestSubscriber.h
 *
 */

#ifndef _TEST_SUBSCRIBER_H_
#define _TEST_SUBSCRIBER_H_

#include <atomic>
#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Types.h>
#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/types/TypeObjectFactory.h>

class TestSubscriber
{
public:

    TestSubscriber();

    virtual ~TestSubscriber();

    //!Initialize the subscriber
    bool init(
            const std::string& topicName,
            int domain,
            eprosima::fastdds::dds::TypeSupport type,
            const eprosima::fastrtps::types::TypeObject* type_object,
            const eprosima::fastrtps::types::TypeIdentifier* type_identifier,
            const eprosima::fastrtps::types::TypeInformation* type_info,
            const std::string& name,
            const eprosima::fastdds::dds::DataRepresentationQosPolicy* dataRepresentationQos,
            const eprosima::fastdds::dds::TypeConsistencyEnforcementQosPolicy* typeConsistencyQos,
            bool use_typelookup = false);

    //!RUN the subscriber
    void run();

    // Auxiliar test methods
    bool isInitialized() const
    {
        return m_bInitialized;
    }

    void waitDiscovery(
            bool expectMatch = true,
            int maxWait = 10);
    void waitTypeDiscovery(
            bool expectMatch = true,
            int maxWait = 10);
    void matched(
            bool unmatched = false);
    bool isMatched()
    {
        return m_subListener.n_matched > 0;
    }

    uint32_t samplesReceived()
    {
        return m_subListener.n_samples;
    }

    eprosima::fastrtps::types::DynamicType_ptr discovered_type() const
    {
        return disc_type_;
    }

    bool register_discovered_type();

    eprosima::fastdds::dds::DataReader* create_datareader();

    void delete_datareader(
            eprosima::fastdds::dds::DataReader* reader);

    eprosima::fastdds::dds::TypeSupport m_Type;

private:

    std::string m_Name;

    eprosima::fastdds::dds::DomainParticipant* mp_participant;
    eprosima::fastdds::dds::Subscriber* mp_subscriber;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::Topic* topic_;
    void* m_Data;
    bool m_bInitialized;
    std::mutex m_mDiscovery;
    std::mutex mtx_type_discovery_;
    std::condition_variable m_cvDiscovery;
    std::condition_variable cv_type_discovery_;
    std::condition_variable cv_;
    eprosima::fastrtps::types::DynamicType_ptr disc_type_;
    eprosima::fastdds::dds::DataReaderQos reader_qos;
    bool using_typelookup_;
    bool tls_callback_called_;
    std::string topic_name_;
    const eprosima::fastdds::dds::DataRepresentationQosPolicy* dataRepresentationQos_;
    const eprosima::fastdds::dds::TypeConsistencyEnforcementQosPolicy* typeConsistencyQos_;

public:

    class PartListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        PartListener(
                TestSubscriber* parent)
            : parent_(parent)
            , discovered_(false)
        {
        }

        ~PartListener() override
        {
        }

        void on_type_discovery(
                eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastrtps::rtps::SampleIdentity& request_sample_id,
                const eprosima::fastcdr::string_255& topic,
                const eprosima::fastrtps::types::TypeIdentifier* identifier,
                const eprosima::fastrtps::types::TypeObject* object,
                eprosima::fastrtps::types::DynamicType_ptr dyn_type) override;

        void on_type_information_received(
                eprosima::fastdds::dds::DomainParticipant* participant,
                const eprosima::fastcdr::string_255 topic_name,
                const eprosima::fastcdr::string_255 type_name,
                const eprosima::fastrtps::types::TypeInformation& type_information) override;

        TestSubscriber* parent_;
        std::atomic<bool> discovered_;

    }
    part_listener_;

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        SubListener()
        {
        }

        SubListener(
                TestSubscriber* parent);

        ~SubListener() override
        {
        }

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        TestSubscriber* mParent;
        int n_matched;
        uint32_t n_samples;
    }
    m_subListener;
};

#endif /* _TEST_SUBSCRIBER_H_ */