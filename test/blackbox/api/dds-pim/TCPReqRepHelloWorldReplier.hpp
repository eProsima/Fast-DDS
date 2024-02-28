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

#include "../../types/HelloWorldPubSubTypes.h"

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

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

    class ReplyListener : public eprosima::fastdds::dds::DataReaderListener
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

        void on_data_available(
                eprosima::fastdds::dds::DataReader* datareader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* /*datareader*/,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            if (0 < info.current_count_change)
            {
                replier_.matched();
            }
            else
            {
                replier_.unmatched();
            }
        }

    private:

        ReplyListener& operator =(
                const ReplyListener&) = delete;

        TCPReqRepHelloWorldReplier& replier_;
    }
    request_listener_;

    class RequestListener : public eprosima::fastdds::dds::DataWriterListener
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

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* /*datawriter*/,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        {
            if (0 < info.current_count_change)
            {
                replier_.matched();
            }
            else
            {
                replier_.unmatched();
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
            const char* certs_folder = nullptr);
    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            eprosima::fastrtps::rtps::SampleIdentity sample_identity,
            uint16_t number);
    void wait_discovery(
            std::chrono::seconds timeout = std::chrono::seconds::zero());
    void wait_unmatched(
            std::chrono::seconds timeout = std::chrono::seconds::zero());
    void matched();
    void unmatched();
    bool is_matched();

    virtual void configDatareader(
            const std::string& suffix)
    {
        std::ostringstream t;

        t << "TCPReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        datareader_topicname_ = t.str();
    }

    virtual void configDatawriter(
            const std::string& suffix)
    {
        std::ostringstream t;

        t << "TCPReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        datawriter_topicname_ = t.str();
    }

protected:

    eprosima::fastdds::dds::DataReaderQos datareader_qos_;
    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;
    std::string datareader_topicname_;
    std::string datawriter_topicname_;

private:

    TCPReqRepHelloWorldReplier& operator =(
            const TCPReqRepHelloWorldReplier&) = delete;

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Topic* request_topic_;
    eprosima::fastdds::dds::Subscriber* request_subscriber_;
    eprosima::fastdds::dds::DataReader* request_datareader_;
    eprosima::fastdds::dds::Topic* reply_topic_;
    eprosima::fastdds::dds::Publisher* reply_publisher_;
    eprosima::fastdds::dds::DataWriter* reply_datawriter_;
    bool initialized_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    std::atomic<unsigned int> matched_;
    eprosima::fastdds::dds::TypeSupport type_;
};

#endif // _TEST_BLACKBOX_TCPReqRepHelloWorldReplier_HPP_
