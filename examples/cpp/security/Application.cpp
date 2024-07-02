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
 * @file Application.cpp
 *
 */

#include "Application.hpp"

#include "CLIParser.hpp"
#include "PublisherApp.hpp"
#include "SubscriberApp.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace security {

//! Factory method to create a publisher or subscriber
std::shared_ptr<Application> Application::make_app(
        const CLIParser::security_config& config,
        const std::string& topic_name)
{
    std::shared_ptr<Application> entity;
    switch (config.entity)
    {
        case CLIParser::EntityKind::PUBLISHER:
            entity = std::make_shared<PublisherApp>(config.pub_config, topic_name);
            break;
        case CLIParser::EntityKind::SUBSCRIBER:
            entity = std::make_shared<SubscriberApp>(config.sub_config, topic_name);
            break;
        case CLIParser::EntityKind::UNDEFINED:
        default:
            throw std::runtime_error("Entity initialization failed");
            break;
    }
    return entity;
}

} // namespace security
} // namespace examples
} // namespace fastdds
} // namespace eprosima
