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

#ifndef UTILS__SYSTEMCOMMANDBUILDER_HPP_
#define UTILS__SYSTEMCOMMANDBUILDER_HPP_

#include <cstdlib>
#include <sstream>

namespace eprosima {

namespace fastdds {

static constexpr const char* FAST_DDS_DEFAULT_CLI_SCRIPT_NAME = "fastdds";
static constexpr const char* FAST_DDS_DEFAULT_CLI_DISCOVERY_VERB = "discovery";
static constexpr const char* FAST_DDS_DEFAULT_CLI_AUTO_VERB = "auto";

/**
 * @brief Class to build and execute system commands.
 */
class SystemCommandBuilder
{
public:

    enum SystemCommandResult
    {
        SUCCESS = 0,
        FAILURE,
        BAD_PARAM,
        INVALID
    };

    SystemCommandBuilder() = default;

    SystemCommandBuilder& executable(
            const std::string& executable)
    {
        command_ << executable;
        return *this;
    }

    SystemCommandBuilder& verb(
            const std::string& verb)
    {
        command_ << " " << verb;
        return *this;
    }

    SystemCommandBuilder& arg(
            const std::string& arg)
    {
        command_ << " " << arg;
        return *this;
    }

    SystemCommandBuilder& value(
            const std::string& value)
    {
        command_ << " " << value;
        return *this;
    }

    int build_and_call()
    {
        return std::system(command_.str().c_str());
    }

private:

    std::stringstream command_;
};

} // namespace fastdds
} // namespace eprosima

#endif  // UTILS__SYSTEMCOMMANDBUILDER_HPP_
