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

#include "FilteringExamplePublisher.h"
#include "FilteringExampleSubscriber.h"


#include <fastrtps/Domain.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima;
using namespace eprosima::fastrtps;

int main(int argc, char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int subscriberType = 1;
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
    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        return 0;
    }


    if (type == 2) // get the subscriber type (fast or slow)
    {
        if (argc > 2) {
            if (strcmp(argv[2], "fast") == 0)
            {
                subscriberType = 1;
            }
            else if (strcmp(argv[2], "slow") == 0)
            {
                subscriberType = 2;
            }
        }
        else
        {
            std::cout << "subscriber type argument needed (fast or slow)" << std::endl;
            return 0;
        }
    }

    // Register the type being used


    switch(type)
    {
        case 1:
            {
                FilteringExamplePublisher mypub;
                if (mypub.init())
                {
                    mypub.run();
                }
                break;
            }
        case 2:
            {
                FilteringExampleSubscriber mysub;
                if (mysub.init(subscriberType))
                {
                    mysub.run();
                }
                break;
            }
    }

    return 0;
}
