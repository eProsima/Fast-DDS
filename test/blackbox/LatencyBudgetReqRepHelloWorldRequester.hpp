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
 * @file LatencyBudgetReqRepHelloWorldRequester.hpp
 *
 */

#ifndef _TEST_BLACKBOX_LatencyBudgetReqRepHelloWorldRequester_HPP_
#define _TEST_BLACKBOX_LatencyBudgetReqRepHelloWorldRequester_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <list>
#include <condition_variable>
#include <asio.hpp>


#if defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

class LatencyBudgetReqRepHelloWorldRequester
{
    public:

        class ReplyListener: public eprosima::fastrtps::SubscriberListener
    {
        public:
            ReplyListener(LatencyBudgetReqRepHelloWorldRequester &requester) : requester_(requester) {};
            ~ReplyListener(){};
            void onNewDataMessage(eprosima::fastrtps::Subscriber *sub);
            void onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*sub*/, eprosima::fastrtps::rtps::MatchingInfo& info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                    requester_.matched();
            }

        private:

            ReplyListener& operator=(const ReplyListener&) = delete;

            LatencyBudgetReqRepHelloWorldRequester &requester_;
    } reply_listener_;

        class RequestListener : public eprosima::fastrtps::PublisherListener
    {
        public:

            RequestListener(LatencyBudgetReqRepHelloWorldRequester &requester) : requester_(requester){};
            ~RequestListener(){};
            void onPublicationMatched(eprosima::fastrtps::Publisher* /*pub*/, eprosima::fastrtps::rtps::MatchingInfo &info)
            {
                if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                    requester_.matched();
            }

        private:

            RequestListener& operator=(const RequestListener&) = delete;

            LatencyBudgetReqRepHelloWorldRequester &requester_;

    } request_listener_;

        LatencyBudgetReqRepHelloWorldRequester();
        virtual ~LatencyBudgetReqRepHelloWorldRequester();
        void init(
            eprosima::fastrtps::Duration_t latency_budget_duration_pub, eprosima::fastrtps::Duration_t latency_budget_duration_sub);
        bool isInitialized() const { return initialized_; }
        void newNumber(eprosima::fastrtps::rtps::SampleIdentity related_sample_identity, uint16_t number);
        void block();
        void wait_discovery(std::chrono::seconds timeout = std::chrono::seconds::zero());
        void matched();
        void send(const uint16_t number);
        bool is_matched();
        const eprosima::fastrtps::Publisher* get_publisher() { return request_publisher_; }
        const eprosima::fastrtps::Subscriber* get_subscriber() { return reply_subscriber_; }
        virtual void configSubscriber(const std::string& suffix)
        {
            sattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

            std::ostringstream t;

            t << "LatencyBudgetReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

            sattr.topic.topicName = t.str();
        };

        virtual void configPublisher(const std::string& suffix)
        {
            puattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

            std::ostringstream t;

            t << "LatencyBudgetReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

            puattr.topic.topicName = t.str();
        }

    protected:
        eprosima::fastrtps::PublisherAttributes puattr;
        eprosima::fastrtps::SubscriberAttributes sattr;
    private:

        LatencyBudgetReqRepHelloWorldRequester& operator=(const LatencyBudgetReqRepHelloWorldRequester&)= delete;

        uint16_t current_number_;
        uint16_t number_received_;
        eprosima::fastrtps::Participant *participant_;
        eprosima::fastrtps::Subscriber *reply_subscriber_;
        eprosima::fastrtps::Publisher *request_publisher_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        std::atomic<unsigned int> matched_;
        HelloWorldType type_;
        eprosima::fastrtps::rtps::SampleIdentity related_sample_identity_;
};

#endif // _TEST_BLACKBOX_LatencyBudgetReqRepHelloWorldRequester_HPP_
