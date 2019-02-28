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

#include "deadlinepayloadPublisher.h"
#include "deadlinepayloadSubscriber.h"


#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>
#include "asio.hpp"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace asio;
int main(int argc, char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int deadline = 1000;
    int sleep = 1000;
    int samples = 0;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
            if (argc > 2)
            {
                deadline = atoi(argv[2]);
                if (argc > 3)
                {
                    sleep = atoi(argv[3]);
                    if (argc > 4)
                    {
                        samples = atoi(argv[4]);
                    }
                }
            }
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
            if (argc > 2)
            {
                deadline = atoi(argv[2]);
            }
        }
    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        return 0;
    }

    // Register the type being used


    switch(type)
    {
        case 1:
            {
                deadlinepayloadPublisher mypub;
                if (mypub.init(deadline))
                {
                    mypub.run(sleep, samples);
                }
                break;
            }
        case 2:
            {
                deadlinepayloadSubscriber mysub;
                if (mysub.init(deadline))
                {
                    mysub.run();
                }
                break;
            }
    }

    return 0;
}
