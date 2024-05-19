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
 * @file HelloWorld_main.cpp
 *
 */

#include <limits>
#include <sstream>

#include "Publisher.h"
#include "Subscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/log/Log.h>

#include <optionparser.hpp>

using eprosima::fastdds::dds::Log;



int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "please choice 1(pub) or 2(sub)";
        return 0;
    }
    char type = *argv[1];
    switch (type)
    {
        case '1':
        {
            student::Publisher mypub;
            if (mypub.init(false))
            {
                mypub.run(1000);
            }
            break;
        }
        case '2':
        {
            student::Subscriber mysub;
            if (mysub.init(false))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
