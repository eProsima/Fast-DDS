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
 * @file HelloWorld_main.cpp
 *
 */

#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

#include "CLIParser.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "SubscriberWaitset.hpp"

using eprosima::fastdds::dds::Log;

int main(
        int argc,
        char** argv)
{
    auto ret = EXIT_SUCCESS;
    CLIParser::hello_world_config config = CLIParser::parse_cli_options(argc, argv);

    if (config.entity == "publisher")
    {
        try
        {
            HelloWorldPublisher hello_world_publisher(config.pub_config);
            hello_world_publisher.run();
        }
        catch (const std::runtime_error& e)
        {
            EPROSIMA_LOG_ERROR(PUBLISHER, e.what());
            ret = EXIT_FAILURE;
        }
    }
    else if (config.entity == "subscriber")
    {
        if (config.sub_config.use_waitset)
        {
            try
            {
                HelloWorldSubscriberWaitset hello_world_subscriber_waitset(config.sub_config);
                hello_world_subscriber_waitset.run();
            }
            catch (const std::runtime_error& e)
            {
                EPROSIMA_LOG_ERROR(SUBSCRIBER, e.what());
                ret = EXIT_FAILURE;
            }
        }
        else
        {
            try
            {
                HelloWorldSubscriber hello_world_subscriber(config.sub_config);
                hello_world_subscriber.run();
            }
            catch (const std::runtime_error& e)
            {
                EPROSIMA_LOG_ERROR(SUBSCRIBER_WAITSET, e.what());
                ret = EXIT_FAILURE;
            }
        }
    }
    // example should never reach this point
    else
    {
        EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown entity " + config.entity);
        CLIParser::print_help(EXIT_FAILURE);
    }

    Log::Reset();
    return ret;
}
