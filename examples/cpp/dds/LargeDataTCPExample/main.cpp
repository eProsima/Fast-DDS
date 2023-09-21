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
 * @file main.cpp
 *
 */

#include <cstdlib>
#include <cstring>

#include <iostream>

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include "LargeDataPublisher.h"
#include "LargeDataSubscriber.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    std::string tcp_type = "client";
    uint16_t pub_frequency = 10;
    uint32_t data_size = 200000;
    if (argc > 1)
    {
        uint8_t argc_idx = 1;
        if (strcmp(argv[argc_idx], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[argc_idx], "subscriber") == 0)
        {
            type = 2;
        }

        argc_idx++;
        if (strcmp(argv[argc_idx], "server") == 0)
        {
            tcp_type = "server";
        }
        else if (strcmp(argv[argc_idx], "client") == 0)
        {
            tcp_type = "client";
        }

        // Sending frequency
        if (type == 1)
        {
            argc_idx++;
            if (argc > 2)
            {
                pub_frequency = static_cast<uint16_t>(atoi(argv[argc_idx]));
            }

            // Size of the data to get sent
            argc_idx++;
            if (argc > 3)
            {
                data_size = static_cast<uint32_t>(atoi(argv[argc_idx]));
            }
        }
    }
    else
    {
        std::cout << "[publisher|subscriber] [client|server] arguments needed" << std::endl;
        Log::Reset();
        return 0;
    }

    switch (type)
    {
        case 1:
        {
            LargeDataPublisher mypub(data_size);
            if (mypub.init(tcp_type))
            {
                mypub.run(pub_frequency);
            }
            break;
        }
        case 2:
        {
            LargeDataSubscriber mysub;
            if (mysub.init(tcp_type))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
