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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/log/Log.h>
#include "types/HelloWorldType.h"

#include <string>

template<typename T>
void print_non_received_messages(const std::list<T>& data, const std::function<void(const T&)>& printer)
{
    if(data.size() != 0)
    {
        std::cout << "Samples not received: ";
        std::for_each(data.begin(), data.end(), printer);
        std::cout << std::endl;
    }
}
std::list<HelloWorld> default_helloworld_data_generator();

int main(void)
{
   const std::string topic_name("Profiling");
   PubSubReader<HelloWorldType> reader(topic_name);
   PubSubWriter<HelloWorldType> writer(topic_name);
   reader.reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();

   writer.init();
   reader.init();

   // Wait for discovery.
   writer.waitDiscovery();
   reader.waitDiscovery();

   auto data = default_helloworld_data_generator();
   
   reader.expected_data(data);
   reader.startReception();

   // Send data
   writer.send(data);

   // Block reader until reception finished or timeout.
   data = reader.block(std::chrono::seconds(5));
   std::function<void(const HelloWorld&)> lambda_print = [](const HelloWorld& hello){std::cout << hello.index() << " ";};
   print_non_received_messages(data, lambda_print);
   Log::Reset();
}

std::list<HelloWorld> default_helloworld_data_generator()
{
    uint16_t index = 1;
    size_t maximum = 100;
    std::list<HelloWorld> returnedValue(maximum);

    std::generate(returnedValue.begin(), returnedValue.end(), [&index] {
            HelloWorld hello;
            hello.index(index);
            std::stringstream ss;
            ss << "HelloWorld " << index;
            hello.message(ss.str());
            ++index;
            return hello;
            });

    return returnedValue;
}

