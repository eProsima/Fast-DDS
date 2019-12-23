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
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/log/Log.h>

using eprosima::fastdds::dds::Log;

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;
    int domain_id = 0;
    int type = 1;
    int count = 20;
    int sleep = 100;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
        }
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "--samples") == 0)
            {
                i++;
                if (argc < i)
                {
                    std::cout << "Samples needs an argument." << std::endl;
                    Log::Reset();
                    return 0;
                }
                count = atoi(argv[i]);
            }
            if (strcmp(argv[i], "--sleep") == 0)
            {
                i++;
                if (argc < i)
                {
                    std::cout << "Sleep needs an argument." << std::endl;
                    Log::Reset();
                    return 0;
                }
                sleep = atoi(argv[i]);
            }
            if (strcmp(argv[i], "--domain_id") == 0)
            {
                i++;
                if (argc < i)
                {
                    std::cout << "Domain Id needs an argument." << std::endl;
                    Log::Reset();
                    return 0;
                }
                domain_id = atoi(argv[i]);
            }
        }

    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        Log::Reset();
        return 0;
    }

    switch (type)
    {
        case 1:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(domain_id))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case 2:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(domain_id))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
