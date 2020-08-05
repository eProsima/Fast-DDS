// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file World_main.cpp
 *
 */

#include "CustomListenerPublisher.h"
#include "CustomListenerSubscriber.h"
#include "CustomListeners.h"


#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/log/Log.h>

using eprosima::fastdds::dds::Log;

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int count = 10;
    int sleep = 100;
    bool use_d;

    if (argc > 2)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
        }

        if (strcmp(argv[2],"1") == 0)
        {
            use_d = true;
        }
        else if (strcmp(argv[2], "0") == 0)
        {
            use_d = false;
        }else{
            std::cout << "Second argument must be 1 (true) or 0 (false)" << std::endl;
            Log::Reset();
            return 0;
        }

    }
    else
    {
        std::cout << "Use: ./DDSCustomListener [subscriber | publisher] [0|1] " << std::endl;
        Log::Reset();
        return 0;
    }

    switch(type)
    {
        case 1:
            {
                CustomListenerPublisher mypub;
                
                if(mypub.init(use_d))
                {
                    mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
                }
                break;
            }
        case 2:
            {
                CustomListenerSubscriber mysub;
                if(mysub.init(use_d))
                {
                    mysub.run();
                }
                break;
            }
    }
    Log::Reset();
    return 0;
}
