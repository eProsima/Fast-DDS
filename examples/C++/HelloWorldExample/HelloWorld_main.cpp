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

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
    std::cout << "Starting "<< std::endl;
    int type = 1;
    int count = 0;
    bool tcp = false;
    if(argc > 1)
    {
        if(strcmp(argv[1],"publisher")==0)
        {
            type = 1;

            for (int i = 2; i < argc; ++i)
            {
                if (strncmp(argv[i], "tcp", strlen(argv[i])) == 0)
                {
                    tcp = true;
                }
                else
                {
                    count = atoi(argv[i]);
                }
            }
        }
        else if(strcmp(argv[1],"subscriber")==0)
        {
            type = 2;
            for (int i = 2; i < argc; ++i)
            {
                if (strncmp(argv[i], "tcp", strlen(argv[i])) == 0)
                {
                    tcp = true;
                }
            }
        }
    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        Log::Reset();
        return 0;
    }


    switch(type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if(mypub.init(tcp))
                {
                    mypub.run(count);
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if(mysub.init(tcp))
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
