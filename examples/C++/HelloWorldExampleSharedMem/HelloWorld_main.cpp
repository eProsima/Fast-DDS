// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
    Log::SetVerbosity(Log::Warning);
    //Log::SetCategoryFilter(std::regex("RTPS_EDP_MATCH|RTPS_PDP_DISCOVERY|RTPS_PARTICIPANT_LISTEN|SHMEM"));
    

    std::cout << "Starting "<< std::endl;
    int type = 1;
    int count = 10;
    long sleep = 100;
    if(argc > 1)
    {
        if(strcmp(argv[1],"publisher")==0)
        {
            type = 1;
            if (argc >= 3)
            {
                count = atoi(argv[2]);
                if (argc == 4)
                {
                    sleep = atoi(argv[3]);
                }
            }
        }
        else if(strcmp(argv[1],"subscriber")==0)
            type = 2;
        else if (strcmp(argv[1], "both") == 0)
            type = 3;
    }
    else
    {
        std::cout << "publisher, subscriber or both argument needed" << std::endl;
        Log::Reset();
        return 0;
    }

    switch(type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if(mypub.init())
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if(mysub.init())
                {
                    mysub.run();
                }
                break;
            }
        case 3:
        {
            std::thread thread_sub([]
                {
                    HelloWorldSubscriber mysub;
                    if (mysub.init())
                    {
                        mysub.run();
                    }
                });

            std::this_thread::sleep_for(std::chrono::seconds(5));

            std::thread thread_pub([&]
                {
                    HelloWorldPublisher mypub;
                    if (mypub.init())
                    {
                        mypub.run(count, sleep);
                    }
                });

            thread_pub.join();
            thread_sub.join();

            break;
        }
    }
    Domain::stopAll();
    Log::Reset();
    return 0;
}
