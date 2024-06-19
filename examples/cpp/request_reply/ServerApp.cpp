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
 * @file ServerApp.cpp
 *
 */

#include "ServerApp.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

ServerApp::ServerApp(
        const CLIParser::config& config,
        const std::string& service_name)
{
    static_cast<void>(config);
    static_cast<void>(service_name);
}

ServerApp::~ServerApp()
{
}

void ServerApp::run()
{
}

void ServerApp::stop()
{
}

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima
