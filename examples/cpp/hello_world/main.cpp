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
 * @file main.cpp
 *
 */

#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "CLIParser.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "SubscriberWaitset.hpp"

using eprosima::fastdds::dds::Log;

std::function<void(std::string)> signal_handler;

int main(
        int argc,
        char** argv)
{
    auto ret = EXIT_SUCCESS;
    HelloWorldPublisher* publisher = nullptr;
    HelloWorldSubscriber* subscriber = nullptr;
    HelloWorldSubscriberWaitset* subscriber_waitset = nullptr;
    std::thread* thread = nullptr;
    const std::string topic_name = "hello_world_topic";
    std::string entity_name = "undefined";
    uint16_t samples = 0;
    eprosima::fastdds::examples::hello_world::CLIParser::hello_world_config config =
            eprosima::fastdds::examples::hello_world::CLIParser::parse_cli_options(argc, argv);

    switch (config.entity)
    {
        case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::PUBLISHER:
            entity_name = "Publisher";
            samples = config.pub_config.samples;
            try
            {
                publisher = new HelloWorldPublisher(config.pub_config, topic_name);
                thread = new std::thread(&HelloWorldPublisher::run, publisher);
            }
            catch (const std::runtime_error& e)
            {
                EPROSIMA_LOG_ERROR(PUBLISHER, e.what());
                ret = EXIT_FAILURE;
            }
            break;
        case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::SUBSCRIBER:
            samples = config.sub_config.samples;
            if (config.sub_config.use_waitset)
            {
                entity_name = "Waitset Subscriber";
                try
                {
                    subscriber_waitset = new HelloWorldSubscriberWaitset(config.sub_config, topic_name);
                    thread = new std::thread(&HelloWorldSubscriberWaitset::run, subscriber_waitset);
                }
                catch (const std::runtime_error& e)
                {
                    EPROSIMA_LOG_ERROR(SUBSCRIBER, e.what());
                    ret = EXIT_FAILURE;
                }
            }
            else
            {
                entity_name = "Subscriber";
                try
                {
                    subscriber = new HelloWorldSubscriber(config.sub_config, topic_name);
                    thread = new std::thread(&HelloWorldSubscriber::run, subscriber);
                }
                catch (const std::runtime_error& e)
                {
                    EPROSIMA_LOG_ERROR(SUBSCRIBER_WAITSET, e.what());
                    ret = EXIT_FAILURE;
                }
            }
            break;
        default:
            EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown entity");
            eprosima::fastdds::examples::hello_world::CLIParser::print_help(EXIT_FAILURE);
            break;
    }

    if (samples == 0)
    {
        std::cout << entity_name << " running. Please press Ctrl+C to stop the "
                  << entity_name << " at any time." << std::endl;
    }
    else
    {
        switch (config.entity)
        {
            case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::PUBLISHER:
                std::cout << entity_name << " running " << samples << " samples. Please press Ctrl+C to stop the "
                          << entity_name << " at any time." << std::endl;
                break;
            case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::SUBSCRIBER:
            default:
                std::cout << entity_name << " running until " << samples << " samples have been received. Please press "
                          << "Ctrl+C to stop the " << entity_name << " at any time." << std::endl;
                break;
        }
    }

    signal_handler = [&](std::string signal)
            {
                std::cout << "\n" << signal << " received, stopping " << entity_name << " execution." << std::endl;
                switch (config.entity)
                {
                    case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::PUBLISHER:
                        if (nullptr != publisher)
                        {
                            publisher->stop();
                        }
                        break;
                    case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::SUBSCRIBER:
                    default:
                        if (config.sub_config.use_waitset)
                        {
                            if (nullptr != subscriber_waitset)
                            {
                                subscriber_waitset->stop();
                            }
                        }
                        else
                        {
                            if (nullptr != subscriber)
                            {
                                subscriber->stop();
                            }
                        }
                        break;
                }
            };
    signal(SIGINT, [](int /*signum*/)
            {
                signal_handler("SIGINT");
            });
    signal(SIGTERM, [](int /*signum*/)
            {
                signal_handler("SIGTERM");
            });
#ifndef _WIN32
    signal(SIGQUIT, [](int /*signum*/)
            {
                signal_handler("SIGQUIT");
            });
    signal(SIGHUP, [](int /*signum*/)
            {
                signal_handler("SIGHUP");
            });
#endif // _WIN32

    thread->join();
    delete thread;
    switch (config.entity)
    {
        case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::PUBLISHER:
            if (nullptr != publisher)
            {
                delete publisher;
            }
            break;
        case eprosima::fastdds::examples::hello_world::CLIParser::EntityKind::SUBSCRIBER:
        default:
            if (config.sub_config.use_waitset)
            {
                if (nullptr != subscriber_waitset)
                {
                    delete subscriber_waitset;
                }
            }
            else
            {
                if (nullptr != subscriber)
                {
                    delete subscriber;
                }
            }
            break;
    }

    Log::Reset();
    return ret;
}
