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

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/rtps/common/Types.h>

#include <condition_variable>

class TestSubscriber
{
public:
    TestSubscriber();

    virtual ~TestSubscriber();

    //!Initialize the subscriber
    bool init(const std::string& topicName, int domain, eprosima::fastrtps::TopicDataType* type,
        const eprosima::fastrtps::types::TypeObject* type_object,
        const eprosima::fastrtps::types::TypeIdentifier* type_identifier,
        const eprosima::fastrtps::types::TypeInformation* type_info,
        const std::string& name,
        const eprosima::fastrtps::DataRepresentationQosPolicy* dataRepresentationQos,
        const eprosima::fastrtps::TypeConsistencyEnforcementQosPolicy* typeConsistencyQos);

    //!RUN the subscriber
    void run();

    // Auxiliar test methods
    bool isInitialized() const { return m_bInitialized; }
    void waitDiscovery(bool expectMatch = true, int maxWait = 10);
    void matched(bool unmatched = false);
    bool isMatched() { return m_subListener.n_matched > 0; }
    uint32_t samplesReceived() { return m_subListener.n_samples; }

    void block(std::function<bool()> checker)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, checker);
    }

    size_t block_for_at_least(size_t at_least)
    {
        block([this, at_least]() -> bool {
                return samplesReceived() >= at_least;
                });
        return samplesReceived();
    }

private:
    std::string m_Name;
    eprosima::fastrtps::TopicDataType *m_Type;
    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Subscriber* mp_subscriber;
    void *m_Data;
    bool m_bInitialized;
    std::mutex m_mDiscovery;
    std::mutex mutex_;
    std::condition_variable m_cvDiscovery;
    std::condition_variable cv_;

public:
    class SubListener :public eprosima::fastrtps::SubscriberListener
    {
    public:
        SubListener() {}
        SubListener(TestSubscriber* parent);

        ~SubListener() {};

        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info);

        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub);

        TestSubscriber* mParent;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        uint32_t n_samples;
    }m_subListener;
};

#endif /* _TEST_SUBSCRIBER_H_ */
