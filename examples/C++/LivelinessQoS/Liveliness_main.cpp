// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Liveliness_main.cpp
 *
 */

#include "LivelinessPublisher.h"
#include "LivelinessPublishers.h"
#include "LivelinessSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/qos/QosPolicies.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

/**
 * @brief Parses command line arguments
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @param type Publisher or subscriber
 * @param first_liveliness_ms Lease duration of the first publisher
 * @param second_liveliness_ms Lease duration of the second publisher
 * @param sleep_ms Writer sleep read from command line arguments (populated if specified)
 * @param samples Number of samples read from command line arguments (populated if specified)
 * @return True if command line arguments were parsed succesfully and execution can continue
 */
bool parse_arguments(
        int argc,
        char** argv,
        int& type,
        int& first_liveliness_ms,
        int& second_liveliness_ms,
        eprosima::fastrtps::LivelinessQosPolicyKind& first_kind,
        eprosima::fastrtps::LivelinessQosPolicyKind& second_kind,
        int& sleep_ms,
        int& samples);

int main(int argc, char** argv)
{
    int type = 1;
    int first_liveliness_ms = 100;
    int second_liveliness_ms = 100;
    int sleep_ms = 1000;
    int count = 10;

    eprosima::fastrtps::LivelinessQosPolicyKind first_kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;
    eprosima::fastrtps::LivelinessQosPolicyKind second_kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;

    if (!parse_arguments(
                argc,
                argv,
                type,
                first_liveliness_ms,
                second_liveliness_ms,
                first_kind,
                second_kind,
                sleep_ms,
                count))
    {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " publisher ";
        std::cout << "[--liveliness <liveliness_ms>] ";
        std::cout << "[--sleep <writer_sleep_ms>] ";
        std::cout << "[--samples <samples>]" << std::endl;

        std::cout << "OR" << std::endl;
        std::cout << argv[0] << " publishers ";
        std::cout << "[--liveliness_first <liveliness_ms>] ";
        std::cout << "[--liveliness_second <liveliness_ms>] ";
        std::cout << "[--kind_first <KIND>] ";
        std::cout << "[--kind_second <KIND>] ";
        std::cout << "[--sleep <writer_sleep_ms>] ";
        std::cout << "[--samples <samples>]" << std::endl;

        std::cout << "OR" << std::endl;
        std::cout << argv[0] << " subscriber ";
        std::cout << "[--liveliness <liveliness_ms>]" << std::endl;
        return 0;
    }

    switch(type)
    {
    case 1:
    {
        LivelinessPublisher mypub;
        if(mypub.init(first_kind, first_liveliness_ms))
        {
            mypub.run(count, sleep_ms);
        }
        break;
    }
    case 2:
    {
        LivelinessPublishers mypub;
        if (mypub.init(
                    first_kind,
                    second_kind,
                    first_liveliness_ms,
                    second_liveliness_ms))
        {
            mypub.run(count, sleep_ms);
        }
        break;
    }
    case 3:
    {
        LivelinessSubscriber mysub;
        if(mysub.init(first_kind, first_liveliness_ms))
        {
            mysub.run();
        }
        break;
    }
}
    Domain::stopAll();
    Log::Reset();
    return 0;
}

bool parse_arguments(
        int argc,
        char** argv,
        int& type,
        int& first_liveliness_ms,
        int& second_liveliness_ms,
        eprosima::fastrtps::LivelinessQosPolicyKind& first_kind,
        eprosima::fastrtps::LivelinessQosPolicyKind& second_kind,
        int& sleep_ms,
        int& samples)
{
    if (argc == 1)
    {
        // No arguments provided
        return false;
    }

    for (int i=0; i<argc; i++)
    {
        if (!strcmp(argv[i], "--help"))
        {
            // --help command found
            return false;
        }
    }

    if (strcmp(argv[1], "publisher") == 0)
    {
        type = 1;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--liveliness"))
            {
                first_liveliness_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--kind"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else
                {
                    std::cout << "Unknown command line value " << argv[count + 1] << " for publisher" << std::endl;
                    return false;
                }
            }
            else if (!strcmp(argv[count], "--sleep"))
            {
                sleep_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--samples"))
            {
                samples = atoi(argv[count + 1]);
            }
            else
            {
                std::cout << "Unknown command line option " << argv[count] << " for publisher" << std::endl;
                return false;
            }
            count = count + 2;
        }
        return true;
    }
    else if (strcmp(argv[1], "publishers") == 0)
    {
        type = 2;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--liveliness_first"))
            {
                first_liveliness_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--liveliness_second"))
            {
                second_liveliness_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--kind_first"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else
                {
                    std::cout << "Unknown command line value " << argv[count + 1] << " for publisher" << std::endl;
                    return false;
                }
            }
            else if (!strcmp(argv[count], "--kind_second"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    second_kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    second_kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else
                {
                    std::cout << "Unknown command line value " << argv[count + 1] << " for publisher" << std::endl;
                    return false;
                }
            }
            else if (!strcmp(argv[count], "--sleep"))
            {
                sleep_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--samples"))
            {
                samples = atoi(argv[count + 1]);
            }
            else
            {
                std::cout << "Unknown command line option " << argv[count] << " for publisher" << std::endl;
                return false;
            }
            count = count + 2;
        }
        return true;
    }

    else if (strcmp(argv[1], "subscriber") == 0)
    {
        type = 3;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--liveliness"))
            {
                first_liveliness_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--kind"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    std::cout << "+++++++ Setting kind to AUTOMATIC" << std::endl;
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    std::cout << "+++++++ Setting kind to MANUAL" << std::endl;
                    first_kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else
                {
                    std::cout << "Unknown command line value " << argv[count + 1] << " for subscriber" << std::endl;
                    return false;
                }
            }
            else
            {
                std::cout << "Unknown command line option " << argv[count] << " for publisher" << std::endl;
                return false;
            }
            count = count + 2;
        }
        return true;
    }

    return false;
}
