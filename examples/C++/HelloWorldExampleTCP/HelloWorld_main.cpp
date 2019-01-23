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

#include <fastrtps/utils/eClock.h>
#include <fastrtps/log/Log.h>

#include <string>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int count = 0;
    long sleep = 100;
    std::string wan_ip;
    int port = 5100;

    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
            if (argc >= 3)
            {
                count = atoi(argv[2]);
                if (argc > 3)
                {
                    sleep = atoi(argv[3]);
                }

                if (argc > 4)
                {
                    wan_ip = std::string(argv[4]);
                }

                if (argc > 5)
                {
                    port = atoi(argv[5]);
                }
            }
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;

            if (argc > 2)
            {
                wan_ip = std::string(argv[2]);
            }

            if (argc > 3)
            {
                port = atoi(argv[3]);
            }
        }
    }
    else
    {
        std::cout << "There was an error with the input arguments." << std::endl << std::endl;
        std::cout << "This example needs at least the argument to set if it is going to work" << std::endl;
        std::cout << "as a 'publisher' or as a 'subscriber'." << std::endl << std::endl;
        
        std::cout << "The publisher is going to work as a TCP server and if the test" << std::endl;
        std::cout << "is through a NAT it must have its public IP in the wan_ip argument." << std::endl << std::endl;
        std::cout << "The optional arguments are: publisher [times] [interval] [wan_ip] [port] " << std::endl;
        std::cout << "\t- times: Number of messages to send (default: unlimited = 0). " << std::endl;
        std::cout << "\t\t If times is set greater than 0, no messages will be sent until a subscriber matches. " << std::endl;
        std::cout << "\t- interval: Milliseconds between messages (default: 100). " << std::endl;
        std::cout << "\t- wap_ip: Public IP Address of the publisher. " << std::endl;
        std::cout << "\t- port: Physical Port to listening incoming connections, this port must be allowed in" << std::endl;
        std::cout << "\t\tthe router of the publisher if the test is going to use WAN IP. " << std::endl << std::endl;
        
        std::cout << "The subscriber is going to work as a TCP client. If the test is through a NAT" << std::endl;
        std::cout << "server_ip must have the WAN IP of the publisher and if the test is on LAN" << std::endl;
        std::cout << "it must have the LAN IP of the publisher" << std::endl << std::endl;
        std::cout << "The optional arguments are: subscriber [server_ip] [port] " << std::endl;
        std::cout << "\t- server_ip: IP Address of the publisher. " << std::endl;
        std::cout << "\t- port: Physical Port where the publisher is listening for connections." << std::endl << std::endl;

        Log::Reset();
        return 0;
    }


    switch (type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if (mypub.init(wan_ip, static_cast<uint16_t>(port)))
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if (mysub.init(wan_ip, static_cast<uint16_t>(port)))
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
