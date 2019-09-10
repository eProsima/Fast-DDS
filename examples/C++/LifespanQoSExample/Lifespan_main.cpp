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
 * @file Lifespan_main.cpp
 *
 */

#include "LifespanPublisher.h"
#include "LifespanSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
    int type = 1;

    // Lifespan
    long lifespan_ms = 500;
    // Sleep time after sending all samples
    long sleep_ms = 2000;
    // Number of samples to send
    int count = 10;
    // Sleep time between samples
    long writer_sleep_ms = 10;

    if(argc > 1)
    {
        if(strcmp(argv[1],"publisher")==0)
        {
            type = 1;
            if (argc > 2)
            {
                lifespan_ms = atoi(argv[2]);
                if (argc > 3)
                {
                    sleep_ms = atoi(argv[3]);
                    if( argc > 4 )
                    {
                        count = atoi(argv[4]);
                        if( argc > 5 )
                        {
                            writer_sleep_ms = atoi(argv[5]);
                        }
                    }
                }
            }
        }
        else if( strcmp(argv[1], "subscriber") == 0 )
        {
            type = 2;
            if( argc > 2 )
            {
                lifespan_ms = atoi(argv[2]);
                if( argc > 3 )
                {
                    sleep_ms = atoi(argv[3]);
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
                LifespanPublisher mypub;
                if( mypub.init(lifespan_ms) )
                {
                    mypub.run(count, writer_sleep_ms, sleep_ms);
                }
                break;
            }
        case 2:
            {
                LifespanSubscriber mysub;
                if( mysub.init(lifespan_ms) )
                {
                    mysub.run(count, sleep_ms);
                }
                break;
            }
    }
    Domain::stopAll();
    Log::Reset();
    return 0;
}
