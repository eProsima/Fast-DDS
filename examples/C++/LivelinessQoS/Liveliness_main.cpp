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
#include "LivelinessSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/qos/QosPolicies.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

//!
//! \brief Parses command line artuments
//! \param argc Number of command line arguments
//! \param argv Array of arguments
//! \param type Publisher or subscriber
//! \param lease_duration_ms Lease duration in ms
//! \param kind Liveliness kind
//! \param sleep_ms Sleep time between consecutive writes
//! \param samples Number of samples to write
//! \return True if command line arguments were parsed correctly
//!
bool parse_arguments(int argc,
        char** argv,
        int& type,
        int& lease_duration_ms,
        LivelinessQosPolicyKind &kind,
        int& sleep_ms,
        int& samples);

int main(int argc, char** argv)
{
    int type = 1;
    int lease_duration_ms = 100;
    int sleep_ms = 1000;
    int count = 10;

    eprosima::fastrtps::LivelinessQosPolicyKind kind = eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS;

    if (!parse_arguments(
                argc,
                argv,
                type,
                lease_duration_ms,
                kind,
                sleep_ms,
                count))
    {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " publisher ";
        std::cout << "[--lease_duration <lease_duration_ms>] ";
        std::cout << "[--kind <AUTOMATIC|MANUAL_BY_PARTICIPANT|MANUAL_BY_TOPIC>]";
        std::cout << "[--sleep <writer_sleep_ms>] ";
        std::cout << "[--samples <samples>]" << std::endl;

        std::cout << "OR" << std::endl;
        std::cout << argv[0] << " subscriber ";
        std::cout << "[--lease_duration <lease_duration_ms>]";
        std::cout << "[--kind <AUTOMATIC|MANUAL_BY_PARTICIPANT|MANUAL_BY_TOPIC>]" << std::endl << std::endl;

        std::cout << "Default values:" << std::endl;
        std::cout << "lease_duration_ms = 100" << std::endl;
        std::cout << "kind = AUTOMATIC" << std::endl;
        std::cout << "writer_sleep_ms = 1000" << std::endl;
        std::cout << "samples = 10" << std::endl;
        return 0;
    }

    switch(type)
    {
    case 1:
    {
        LivelinessPublisher mypub;
        if(mypub.init(kind, lease_duration_ms))
        {
            mypub.run(count, sleep_ms);
        }
        break;
    }
    case 2:
    {
        LivelinessSubscriber mysub;
        if(mysub.init(kind, lease_duration_ms))
        {
            mysub.run();
        }
        break;
    }
}
    Domain::stopAll();
    eprosima::fastdds::dds::Log::Reset();
    return 0;
}

bool parse_arguments(
        int argc,
        char** argv,
        int& type,
        int& lease_duration_ms,
        eprosima::fastrtps::LivelinessQosPolicyKind& kind,
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
            if (!strcmp(argv[count], "--lease_duration"))
            {
                lease_duration_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--kind"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_TOPIC"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
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
        type = 2;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--lease_duration"))
            {
                lease_duration_ms = atoi(argv[count + 1]);
            }
            else if (!strcmp(argv[count], "--kind"))
            {
                if (!strcmp(argv[count + 1], "AUTOMATIC"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_PARTICIPANT"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                }
                else if(!strcmp(argv[count + 1], "MANUAL_BY_TOPIC"))
                {
                    kind = eprosima::fastrtps::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
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
