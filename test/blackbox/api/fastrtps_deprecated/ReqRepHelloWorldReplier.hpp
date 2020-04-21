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
 * @file ReqRepHelloWorldReplier.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_
#define _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_

#include "../../types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <list>
#include <condition_variable>

#if defined(_WIN32)
#define GET_PID _getpid
#include <process.h>
#else
#define GET_PID getpid
#endif



class ReqRepHelloWorldReplier
{
public:

    class ReplyListener : public eprosima::fastrtps::SubscriberListener
    {
public:

        ReplyListener(
                ReqRepHelloWorldReplier& replier)
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

        ReqRepHelloWorldReplier& replier_;
    } request_listener_;

    class RequestListener : public eprosima::fastrtps::PublisherListener
    {
public:

        RequestListener(
                ReqRepHelloWorldReplier& replier)
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

        ReqRepHelloWorldReplier& replier_;

    } reply_listener_;

    ReqRepHelloWorldReplier();
    virtual ~ReqRepHelloWorldReplier();
    void init();
    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            eprosima::fastrtps::rtps::SampleIdentity sample_identity,
            uint16_t number);
    void wait_discovery();
    void matched();
    virtual void configSubscriber(
            const std::string& suffix) = 0;
    virtual void configPublisher(
            const std::string& suffix) = 0;

protected:

    eprosima::fastrtps::SubscriberAttributes sattr;
    eprosima::fastrtps::PublisherAttributes puattr;

private:

    ReqRepHelloWorldReplier& operator =(
            const ReqRepHelloWorldReplier&) = delete;

    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Subscriber* request_subscriber_;
    eprosima::fastrtps::Publisher* reply_publisher_;
    bool initialized_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    unsigned int matched_;
    HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_
