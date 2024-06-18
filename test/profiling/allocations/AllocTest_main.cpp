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
 * @file AllocTest_main.cpp
 *
 */

#include <fastdds/dds/log/Log.hpp>

#include "AllocTestPublisher.hpp"
#include "AllocTestSubscriber.hpp"

using namespace eprosima;
using namespace fastdds;
using namespace rtps;

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;
    int type = 1;
    int domain = 1;
    bool wait_unmatch = false;
    const char* profile = "tl_be";
    std::string outputFile = "";
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

        profile = argv[2];

        wait_unmatch = (argc > 3) && (strcmp(argv[3], "true") == 0);
        if (argc > 4)
        {
            domain = atoi(argv[4]);
        }

        if (argc > 5)
        {
            outputFile = argv[5];
        }
    }
    else
    {
        std::cout
            << "Syntax is AllocationTestExample <kind> <profile>, where:" << std::endl
            << "    kind:" << std::endl
            << "        publisher OR subscriber" << std::endl
            << "    profile:" << std::endl
            << "        tl_be: transient-local best-effort" << std::endl
            << "        tl_re: transient-local reliable" << std::endl
            << "        vo_be: volatile best-effort" << std::endl
            << "        vo_re: volatile reliable" << std::endl;
        eprosima::fastdds::dds::Log::Reset();
        return 0;
    }


    switch (type)
    {
        case 1:
        {
            AllocTestPublisher mypub;
            if (mypub.init(profile, domain, outputFile))
            {
                mypub.run(60, wait_unmatch);
            }
            break;
        }
        case 2:
        {
            AllocTestSubscriber mysub;
            if (mysub.init(profile, domain, outputFile))
            {
                mysub.run(wait_unmatch);
            }
            break;
        }
    }

    eprosima::fastdds::dds::Log::Reset();

    return 0;
}
