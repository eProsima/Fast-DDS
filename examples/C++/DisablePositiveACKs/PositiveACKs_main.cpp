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
 * @file PositiveACKs_main.cpp
 *
 */

#include "PositiveACKsPublisher.h"
#include "PositiveACKsSubscriber.h"

#include <fastrtps/Domain.h>

#include <fastrtps/utils/eClock.h>
#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
int main(int argc, char** argv)
{
    int type = 1;

    // >0 to use disable positive acks
    long use_disable_positive_acks = 1;
    // Keep duration in milliseconds
    long keep_duration_ms = 5000;
    // Sleep time between samples
    long writer_sleep_ms = 1000;
    // Number of samples to send
    long count = 20;

    if(argc > 1)
    {
        if(strcmp(argv[1],"publisher")==0)
        {
            type = 1;
            if (argc > 2)
            {
                use_disable_positive_acks = atoi(argv[2]);
                if (argc > 3)
                {
                    keep_duration_ms = atoi(argv[3]);
                    if( argc > 4 )
                    {
                        writer_sleep_ms = atoi(argv[4]);
                        if( argc > 5 )
                        {
                            count = atoi(argv[5]);
                        }
                    }
                }
            }
        }
        else if( strcmp(argv[1], "subscriber") == 0 )
        {
            type = 2;
            if (argc > 2)
            {
                use_disable_positive_acks = atoi(argv[2]);
                if (argc > 3)
                {
                    count = atoi(argv[2]);
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
                PositiveACKsPublisher mypub;
                if( mypub.init(use_disable_positive_acks > 0, keep_duration_ms) )
                {
                    mypub.run(count, writer_sleep_ms);
                }
                break;
            }
        case 2:
            {
                PositiveACKsSubscriber mysub;
                if( mysub.init(use_disable_positive_acks) )
                {
                    mysub.run(count);
                }
                break;
            }
    }
    Domain::stopAll();
    Log::Reset();
    return 0;
}
