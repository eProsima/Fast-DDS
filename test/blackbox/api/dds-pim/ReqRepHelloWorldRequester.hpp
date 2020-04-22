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
 * @file ReqRepHelloWorldRequester.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
#define _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_

#include "../../types/HelloWorldType.h"

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
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif

namespace eprosima {
namespace fastdds {
namespace dds {
class DomainParticipant;
class Topic;
class Subscriber;
class DataReader;
class Publisher;
class DataWriter;
}
}
}

class ReqRepHelloWorldRequester
{
public:

    class ReplyListener : public eprosima::fastdds::dds::DataReaderListener
    {
public:

        ReplyListener(
                ReqRepHelloWorldRequester& requester)
            : requester_(requester)
        {
        }

        ~ReplyListener()
        {
        }

        void on_data_available(
                eprosima::fastdds::dds::DataReader* datareader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader*,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override
        {
            if (0 < info.current_count_change)
            {
                requester_.matched();
            }
        }

private:

        ReplyListener& operator =(
                const ReplyListener&) = delete;

        ReqRepHelloWorldRequester& requester_;
    } reply_listener_;

    class RequestListener : public eprosima::fastdds::dds::DataWriterListener
    {
public:

        RequestListener(
                ReqRepHelloWorldRequester& requester)
            : requester_(requester)
        {
        }

        ~RequestListener()
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter*,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override
        {
            if (0 < info.current_count_change)
            {
                requester_.matched();
            }
        }

private:

        RequestListener& operator =(
                const RequestListener&) = delete;

        ReqRepHelloWorldRequester& requester_;

    } request_listener_;

    ReqRepHelloWorldRequester();
    virtual ~ReqRepHelloWorldRequester();
    void init();
    void init_with_latency(
            const eprosima::fastrtps::Duration_t& latency_budget_duration_pub,
            const eprosima::fastrtps::Duration_t& latency_budget_duration_sub);
    bool isInitialized() const
    {
        return initialized_;
    }

    void newNumber(
            eprosima::fastrtps::rtps::SampleIdentity related_sample_identity,
            uint16_t number);
    void block(
            const std::chrono::seconds& seconds);
    void wait_discovery();
    void matched();
    void send(
            const uint16_t number);
    const eprosima::fastrtps::Duration_t datawriter_latency_budget_duration() const
    {
        return request_datawriter_->get_qos().latency_budget().duration;
    }

    const eprosima::fastrtps::Duration_t datareader_latency_budget_duration() const
    {
        return reply_datareader_->get_qos().latency_budget().duration;
    }

    virtual void configDatareader(
            const std::string& suffix)
    {
        std::ostringstream t;

        t << "ReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        datareader_topicname_ = t.str();
    }

    virtual void configDatawriter(
            const std::string& suffix)
    {

        std::ostringstream t;

        t << "ReqRepHelloworld_" << asio::ip::host_name() << "_" << GET_PID() << "_" << suffix;

        datawriter_topicname_ = t.str();
    }

protected:

    eprosima::fastdds::dds::DataWriterQos datawriter_qos_;
    eprosima::fastdds::dds::DataReaderQos datareader_qos_;
    std::string datareader_topicname_;
    std::string datawriter_topicname_;

private:

    ReqRepHelloWorldRequester& operator =(
            const ReqRepHelloWorldRequester&) = delete;

    uint16_t current_number_;
    uint16_t number_received_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Topic* reply_topic_;
    eprosima::fastdds::dds::Subscriber* reply_subscriber_;
    eprosima::fastdds::dds::DataReader* reply_datareader_;
    eprosima::fastdds::dds::Topic* request_topic_;
    eprosima::fastdds::dds::Publisher* request_publisher_;
    eprosima::fastdds::dds::DataWriter* request_datawriter_;
    bool initialized_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::mutex mutexDiscovery_;
    std::condition_variable cvDiscovery_;
    unsigned int matched_;
    eprosima::fastdds::dds::TypeSupport type_;
    eprosima::fastrtps::rtps::SampleIdentity related_sample_identity_;
    eprosima::fastrtps::rtps::SampleIdentity received_sample_identity_;
};

#endif // _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
