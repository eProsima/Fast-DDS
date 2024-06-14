// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file main.cpp
 *
 */

#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <optionparser.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"

using namespace eprosima::fastdds::examples::content_filter;
using eprosima::fastdds::dds::Log;
namespace option = eprosima::option;

std::function<void(int)> stop_app_handler;
void signal_handler(
        int signum)
{
    stop_app_handler(signum);
}

int main(
        int argc,
        char** argv)
{
    auto ret = EXIT_SUCCESS;
    const std::string topic_name = "content_filter_topic";
    CLIParser::hello_world_config config = CLIParser::parse_cli_options(argc, argv);

    // Initialize variables with default values
    uint16_t samples = 0;
    uint16_t type = 1;
    uint16_t sleep = 100;

    switch (config.entity)
    {
        case CLIParser::EntityKind::PUBLISHER:
            samples = config.pub_config.samples;
            break;
        default:
            break;
    }

    std::string app_name = CLIParser::parse_entity_kind(config.entity);
    std::shared_ptr<Application> app;

    try
    {
        app = Application::make_app(config, topic_name);
    }
    catch (const std::runtime_error& e)
    {
        EPROSIMA_LOG_ERROR(app_name, e.what());
        ret = EXIT_FAILURE;
    }

    if (EXIT_FAILURE != ret)
    {
        std::thread thread(&Application::run, app);

        if (samples == 0)
        {
            std::cout << app_name << " running. Please press Ctrl+C to stop the "
                      << app_name << " at any time." << std::endl;
        }
        else
        {
            std::cout << app_name << " running for " << samples << " samples. Please press Ctrl+C to stop the "
                      << app_name << " at any time." << std::endl;
        }

        stop_app_handler = [&](int signum)
                {
                    std::cout << "\n" << CLIParser::parse_signal(signum) << " received, stopping " << app_name
                              << " execution." << std::endl;
                    app->stop();
                };

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #ifndef _WIN32
        signal(SIGQUIT, signal_handler);
        signal(SIGHUP, signal_handler);
    #endif // _WIN32

        thread.join();
    }

    // Flush Fast DDS Log before closing application
    Log::Reset();
    return ret;
}
