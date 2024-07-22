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
#include "ReaderApp.hpp"
#include "WriterApp.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

//! Factory method to create a publisher or subscriber
std::shared_ptr<Application> Application::make_app(
        const CLIParser::rtps_config& config,
        const std::string& topic_name)
{
    std::shared_ptr<Application> entity;
    switch (config.entity)
    {
        case CLIParser::EntityKind::WRITER:
            entity = std::make_shared<WriterApp>(config, topic_name);
            break;
        case CLIParser::EntityKind::READER:
            entity = std::make_shared<ReaderApp>(config, topic_name);
            break;
        case CLIParser::EntityKind::UNDEFINED:
        default:
            throw std::runtime_error("Entity initialization failed");
            break;
    }
    return entity;
}

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima
