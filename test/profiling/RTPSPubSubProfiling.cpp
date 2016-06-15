#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <fastrtps/rtps/RTPSDomain.h>
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

