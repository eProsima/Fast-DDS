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

#include "RTPSAsNonReliableSocketWriter.hpp"
#include "RTPSAsNonReliableSocketReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/log/Log.h>

#include <thread>
#include <chrono>
#include <string>


int main(int argc, char* argv[])
{

    uint16_t nmsgs = 100;
    if (argc == 2)
      nmsgs = std::stoi(argv[1], nullptr, 10);

    RTPSAsNonReliableSocketReader reader;
    RTPSAsNonReliableSocketWriter writer;
    std::string ip("239.255.1.4");
    const uint32_t port = 22222;
    
    reader.init(ip, port, nmsgs);

    if(!reader.isInitialized())
        return 1;

    writer.init(ip, port);

    if(!writer.isInitialized())
        return 1;

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    writer.send(msgs);
    reader.block(*msgs.rbegin(), std::chrono::seconds(1));

    while(!reader.getNonReceivedMessages().empty())
       std::this_thread::sleep_for(std::chrono::seconds(1));

    Log::Reset();
    return 0;
}
