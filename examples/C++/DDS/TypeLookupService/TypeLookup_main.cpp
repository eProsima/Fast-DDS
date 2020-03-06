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
 * @file TypeLookup_main.cpp
 *
 */

#include "TypeLookupPublisher.h"
#include "TypeLookupSubscriber.h"

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int count = 5;
    long sleep = 100;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
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
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
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
                TypeLookupPublisher mypub;
                if (mypub.init())
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                TypeLookupSubscriber mysub;
                if (mysub.init())
                {
                    mysub.run();
                }
                break;
            }
    }
    Log::Reset();
    return 0;
}
