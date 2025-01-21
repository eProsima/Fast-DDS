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

#include "CliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

int main (
        int argc,
        char* argv[])
{
    using ToolCommand = fds::ToolCommand;

    // Skip program name if present
    argc -= (argc > 0);
    argv += (argc > 0);

    // Command index is provided by python tool, so no need to check uint16_t limits
    uint16_t command_int = std::stoi(argv[0]);

    // Skip command index argv[0] if present
    argc -= (argc > 0);
    argv += (argc > 0);

    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
    eprosima::fastdds::dds::CliDiscoveryManager cli_manager;

    switch (command_int)
    {
        case ToolCommand::AUTO:
            return cli_manager.fastdds_discovery_auto(options, parse);
            break;
        case ToolCommand::START:
            return cli_manager.fastdds_discovery_start(options, parse);
            break;
        case ToolCommand::STOP:
            return cli_manager.fastdds_discovery_stop(options, parse);
            break;
        case ToolCommand::ADD:
            return cli_manager.fastdds_discovery_add(options, parse);
            break;
        case ToolCommand::SET:
            return cli_manager.fastdds_discovery_set(options, parse);
            break;
        case ToolCommand::LIST:
            return cli_manager.fastdds_discovery_list(options, parse);
            break;
        case ToolCommand::INFO:
            return cli_manager.fastdds_discovery_info(options, parse);
            break;
        case ToolCommand::SERVER:
            return cli_manager.fastdds_discovery_server(options, parse);
            break;

        default:
            break;
    }
}