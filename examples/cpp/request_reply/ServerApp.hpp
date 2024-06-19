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

#include <string>

#include "Application.hpp"
#include "CLIParser.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

class ServerApp : public Application
{
public:

    ServerApp(
            const CLIParser::config& config,
            const std::string& service_name);

    ~ServerApp();

    //! Run server
    void run() override;

    //! Stop server
    void stop() override;
};

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif /* FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__SERVERAPP_HPP */
