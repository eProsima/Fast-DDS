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
 * @file BenchMark_main.cpp
 *
 */

#include "BenchmarkPublisher.h"
#include "BenchmarkSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(int argc, char** argv)
{
    int type = 1;
    int transport = 1;
    bool paramsOk = true;
	std::string topic = "BenchMarkTopicTCP";
	int domain = 0;
	int size = 0;
	int test_time = 10000;
	int tick_time = 100;
	int wait_time = 1000;
	ReliabilityQosPolicyKind kind = RELIABLE_RELIABILITY_QOS;

    if(argc >= 2)
    {
        if(strcmp(argv[1],"publisher")==0)
        {
            type = 1;
        }
        else if(strcmp(argv[1],"subscriber")==0)
        {
            type = 2;
        }
        else
        {
            paramsOk = false;
        }

        if (paramsOk && argc > 2)
        {
            if (strcmp(argv[2], "udp") == 0)
            {
                transport = 1;
            }
            else if (strcmp(argv[2], "tcp") == 0)
            {
                transport = 2;
            }
            else if (strcmp(argv[2], "udp6") == 0)
            {
                transport = 3;
            }
            else if (strcmp(argv[2], "tcp6") == 0)
            {
                transport = 4;
            }
            else
            {
                paramsOk = false;
            }
        }
		if (argc > 3)
		{
			if (argc % 2 == 0)
			{
				paramsOk = false;
			}
			for (int i = 3; i < argc; i += 2)
			{
				if (strcmp(argv[i], "-topic") == 0)
				{
					topic = argv[i + 1];
				}
				else if (strcmp(argv[i], "-domain") == 0)
				{
					domain = atoi(argv[i + 1]);
				}
				else if (strcmp(argv[i], "-time") == 0)
				{
					test_time = atoi(argv[i + 1]);
				}
				else if (strcmp(argv[i], "-size") == 0)
				{
					if (strcmp(argv[i + 1], "none") == 0)
					{
						size = 0;
					}
					else if (strcmp(argv[i + 1], "small") == 0)
					{
						size = 1;
					}
					else if (strcmp(argv[i + 1], "medium") == 0)
					{
						size = 2;
					}
					else if (strcmp(argv[i + 1], "big") == 0)
					{
						size = 3;
					}
					else
					{
						paramsOk = false;
					}
				}
				else if (strcmp(argv[i], "-reliable") == 0)
				{
					if (strcmp(argv[i + 1], "true") == 0)
					{
						kind = RELIABLE_RELIABILITY_QOS;
					}
					else if (strcmp(argv[i + 1], "false") == 0)
					{
						kind = BEST_EFFORT_RELIABILITY_QOS;
					}
					else
					{
						paramsOk = false;
					}
				}
				else if (strcmp(argv[i], "-tick") == 0)
				{
					tick_time = atoi(argv[i + 1]);
				}
				else if (strcmp(argv[i], "-wait") == 0)
				{
					wait_time = atoi(argv[i + 1]);
				}
				else
				{
					paramsOk = false;
				}
			}
		}
    }
	else
	{
		paramsOk = false;
	}

    if (!paramsOk)
    {
		std::cout << "publisher OR subscriber argument needed" << std::endl;
		std::cout << "tcp OR udp argument needed" << std::endl;
		std::cout << "-------------------------------------------------------------------- " << std::endl;
		std::cout << "Common optional arguments: [-topic] [-domain][-reliable] " << std::endl;
		std::cout << "\t-topic: Prefix topic for the communication " << std::endl;
		std::cout << "\t-domain: Domain id " << std::endl;
		std::cout << "\t-reliable: Sets if the communication is going to be Reliable or Best_Effort. Values [true] [false] " << std::endl;
		std::cout << "\t-size: Size of the message. Values [none] [small] [medium] [big] " << std::endl;
		std::cout << "\t\t: none. The message only contains an int value" << std::endl;
		std::cout << "\t\t: small. The message contains an int value and an array of 16Kb " << std::endl;
		std::cout << "\t\t: medium. The message contains an int value and an array of 512Kb" << std::endl;
		std::cout << "\t\t: big. The message contains an int value and an array of 8Mb" << std::endl;
		std::cout << "-------------------------------------------------------------------- " << std::endl;
		std::cout << "publisher has an optional argument: [time] " << std::endl;
		std::cout << "\t-wait: Milliseconds before starting the sampling. " << std::endl;
		std::cout << "\t-time: Milliseconds that the test is going to run. " << std::endl;
		std::cout << "\t-tick: Milliseconds to take samples of the performance. " << std::endl;
		std::cout << "-------------------------------------------------------------------- " << std::endl;
        eprosima::fastdds::dds::Log::Reset();
        return 0;
    }

    switch(type)
    {
        case 1:
            {
				BenchMarkPublisher* mypub = new BenchMarkPublisher();
                if(mypub->init(transport, kind, test_time, tick_time, wait_time, topic, domain, size))
                {
                    mypub->run();
                }
				delete mypub;
                break;
            }
        case 2:
            {
                BenchMarkSubscriber* mysub = new BenchMarkSubscriber();
                if(mysub->init(transport, kind, topic, domain, size))
                {
                    mysub->run();
                }
				delete mysub;
                break;
            }
    }
    Domain::stopAll();
    eprosima::fastdds::dds::Log::Reset();
    return 0;
}
