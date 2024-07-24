// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ServerApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__SERVERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__SERVERAPP_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "app_utils.hpp"
#include "Application.hpp"
#include "types/CalculatorPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

using namespace eprosima::fastdds::dds;

class ServerApp : public Application, public DomainParticipantListener
{
public:

    ServerApp(
            const std::string& service_name);

    ~ServerApp();

    //! Run server
    void run() override;

    //! Stop server
    void stop() override;

    //! Participant discovery method
    void on_participant_discovery(
            DomainParticipant* participant,
            rtps::ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override;

    //! Publication matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Subscription matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Request received method
    void on_data_available(
            DataReader* reader) override;

private:

    void create_participant();

    template<typename TypeSupportClass>
    Topic* create_topic(
            const std::string& topic_name,
            TypeSupport& type);

    void create_request_entities(
            const std::string& service_name);

    void create_reply_entities(
            const std::string& service_name);

    bool is_stopped();

    void reply_routine();

    bool calculate(
            const CalculatorRequestType& request,
            std::int32_t& result);

    DomainParticipant* participant_;

    TypeSupport request_type_;

    Topic* request_topic_;

    Subscriber* subscriber_;

    DataReader* request_reader_;

    TypeSupport reply_type_;

    Topic* reply_topic_;

    Publisher* publisher_;

    DataWriter* reply_writer_;

    std::mutex mtx_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;

    RemoteClientMatchedStatus client_matched_status_;

    struct Request
    {
        SampleInfo info;
        std::shared_ptr<CalculatorRequestType> request;
    };

    std::queue<Request> requests_;

    std::thread reply_thread_;

};

template<>
Topic* ServerApp::create_topic<CalculatorRequestTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type);

template<>
Topic* ServerApp::create_topic<CalculatorReplyTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type);

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif /* FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__SERVERAPP_HPP */
