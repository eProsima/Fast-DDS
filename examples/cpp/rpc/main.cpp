// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <condition_variable>
#include <csignal>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <atomic>

#include "app_utils.hpp"
#include "Application.hpp"
#include "CLIParser.hpp"

using eprosima::fastdds::dds::Log;

using namespace eprosima::fastdds::examples::rpc;

std::function<void(int)> stop_app_handler;
std::mutex stop_app_mtx;
std::condition_variable stop_app_cv;
std::atomic<bool> stop_requested {false};

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
    const std::string service_name = "Calculator_Service";
    CLIParser::config config = CLIParser::parse_cli_options(argc, argv);
    std::string app_name = CLIParser::parse_entity_kind(config.entity);
    std::shared_ptr<Application> app;

    try
    {
        app = Application::make_app(config, service_name);
    }
    catch (const std::runtime_error& e)
    {
        client_server_error("main", e.what());
        ret = EXIT_FAILURE;
    }

    if (EXIT_FAILURE != ret)
    {
        std::thread thread(&Application::run, app);

        stop_app_handler = [&](int signum)
                {
                    client_server_info("main",
                            CLIParser::parse_signal(signum) << " received, stopping " << app_name << " execution.");

                    stop_requested.store(true);
                    stop_app_cv.notify_all();
                };

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #ifndef _WIN32
        signal(SIGQUIT, signal_handler);
        signal(SIGHUP, signal_handler);
    #endif // _WIN32

        client_server_info("main",
                app_name << " running. Please press Ctrl+C to stop the " << app_name << " at any time.");

        thread.join();
    }

    return ret;
}
