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

#ifndef FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__SERVERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__SERVERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace discovery_server {

class ServerApp : public Application, public DomainParticipantListener
{
public:

    ServerApp(
            const CLIParser::server_config& config);

    ~ServerApp();

    //! Publisher matched method
    void on_participant_discovery(
            DomainParticipant* participant,
            fastdds::rtps::ParticipantDiscoveryStatus status,
            const fastdds::rtps::ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override;

    //! Run publisher
    void run() override;

    //! Stop publisher
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    DomainParticipant* participant_;

    int16_t matched_;

    std::mutex mutex_;

    uint16_t timeout_;

    std::chrono::steady_clock::time_point start_time_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;
};

} // namespace discovery_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__SERVERAPP_HPP
