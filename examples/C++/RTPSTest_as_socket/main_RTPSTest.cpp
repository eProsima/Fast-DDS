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

#include <stdio.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <sstream>

#include "fastrtps/log/Log.h"
#include "fastrtps/rtps/RTPSDomain.h"

#include "TestWriterSocket.h"
#include "TestReaderSocket.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;


int main(int argc, char** argv){

    std::cout << "Starting" << std::endl;
    int type;
    if(argc > 1)
    {
        if(strcmp(argv[1],"writer")==0)
            type = 1;
        else if(strcmp(argv[1],"reader")==0)
            type = 2;
        else
        {
            std::cout << "NEEDS writer OR reader as first argument"<<std::endl;
            return 0;
        }
    }
    else
    {
        std::cout << "NEEDS writer OR reader ARGUMENT"<<std::endl;
        std::cout << "RTPSTest writer"<<std::endl;
        std::cout << "RTPSTest reader" <<std::endl;
        return 0;
    }
    switch (type)
    {
        case 1:
            {
                TestWriterSocket TW;
                if(TW.init("239.255.1.4",22222))
                    TW.run(10);
                break;
            }
        case 2:
            {
                TestReaderSocket TR;
                if(TR.init("239.255.1.4",22222))
                    TR.run();
                break;
            }
    }

    RTPSDomain::stopAll();
    std::cout << "EVERYTHING STOPPED FINE"<<std::endl;

    return 0;
}



