#include "RTPSAsNonReliableSocketWriter.hpp"
#include "RTPSAsNonReliableSocketReader.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/utils/RTPSLog.h>

#include <thread>
#include <chrono>
#include <string>


int main(int argc, char* argv[])
{
    eprosima::Log::setVerbosity(eprosima::LOG_VERBOSITY_LVL::VERB_ERROR);
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

    return 0;
}
