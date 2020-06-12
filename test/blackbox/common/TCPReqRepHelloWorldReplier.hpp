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
 * @file TCPReqRepHelloWorldReplier.hpp
 *
 */

#ifndef _TEST_BLACKBOX_TCPReqRepHelloWorldReplier_HPP_
#define _TEST_BLACKBOX_TCPReqRepHelloWorldReplier_HPP_

#include "../types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <list>
#include <condition_variable>
#include <asio.hpp>

#if defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#endif // if defined(_WIN32)



class TCPReqRepHelloWorldReplier
{
public:

    class ReplyListener : public eprosima::fastrtps::SubscriberListener
    {
    public:

        ReplyListener(
                TCPReqRepHelloWorldReplier& replier)
            : replier_(replier)
        {
        }

        ~ReplyListener()
        {
        }

        void onNewDataMessage(
                eprosima::fastrtps::Subscriber* sub);
        void onSubscriptionMatched(
                eprosima::fastrtps::Subscriber* /*sub*/,
                eprosima::fastrtps::rtps::MatchingInfo& info)
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                replier_.matched();
            }
        }

    private:

        ReplyListener& operator =(
                const ReplyListener&) = delete;

        TCPReqRepHelloWorldReplier& replier_;
    }
    request_listener_;

    class RequestListener : public eprosima::fastrtps::PublisherListener
    {
    public:

        RequestListener(
                TCPReqRepHelloWorldReplier& replier)
            : replier_(replier)
        {
        }

        ~RequestListener()
        {
        }

        void onPublicationMatched(
                eprosima::fastrtps::Publisher* /*pub*/,
                eprosima::fastrtps::rtps::MatchingInfo& info)
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                replier_.matched();
            }
        }

    private:

        RequestListener& operator =(
                const RequestListener&) = delete;

        TCPReqRepHelloWorldReplier& replier_;

    }
    reply_listener_;

    TCPReqRepHelloWorldReplier();
    virtual ~TCPReqRepHelloWorldReplier();
    void init(
            int participantId,
            int domainId,
            uint16_t listeningPort,
            uint32_t maxInitialPeer = 0,
            const char* certs_path = nullptr);
    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            eprosima::fastrtps::rtps::SampleIdentity sample_identity,
            uint16_t number);
    void wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero());
    void matched();
    bool is_matched();

    virtual void configSubscriber(
            const std::string& suffix)
    {
        sattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

        std::ostringstream t;

        t << "TCPReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        sattr.topic.topicName = t.str();
    }

    virtual void configPublisher(
            const std::string& suffix)
    {
        puattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

        // Increase default max_blocking_time to 1 second, as our CI infrastructure shows some
        // big CPU overhead sometimes
        puattr.qos.m_reliability.max_blocking_time.seconds = 1;
        puattr.qos.m_reliability.max_blocking_time.nanosec = 0;

        std::ostringstream t;

        t << "TCPReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        puattr.topic.topicName = t.str();
    }

protected:

    eprosima::fastrtps::SubscriberAttributes sattr;
    eprosima::fastrtps::PublisherAttributes puattr;

private:

    TCPReqRepHelloWorldReplier& operator =(
            const TCPReqRepHelloWorldReplier&) = delete;

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Subscriber* request_subscriber_;
    eprosima::fastrtps::Publisher* reply_publisher_;
    bool initialized_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    std::atomic<unsigned int> matched_;
    HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_TCPReqRepHelloWorldReplier_HPP_
