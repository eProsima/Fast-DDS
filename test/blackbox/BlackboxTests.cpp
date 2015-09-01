#include "RTPSAsNonReliableSocketReader.hpp"
#include "RTPSAsNonReliableSocketWriter.hpp"
#include "RTPSAsReliableSocketReader.hpp"
#include "RTPSAsReliableSocketWriter.hpp"
#include "RTPSAsNonReliableWithRegistrationReader.hpp"
#include "RTPSAsNonReliableWithRegistrationWriter.hpp"
#include "RTPSAsReliableWithRegistrationReader.hpp"
#include "RTPSAsReliableWithRegistrationWriter.hpp"
#include "PubSubAsNonReliableHelloWorldReader.hpp"
#include "PubSubAsNonReliableHelloWorldWriter.hpp"

#include <fastrtps/rtps/RTPSDomain.h>

#include <gtest/gtest.h>

class BlackboxEnvironment : public ::testing::Environment
{
    public:

        void SetUp() {}

        void TearDown()
        {
            eprosima::fastrtps::rtps::RTPSDomain::stopAll();
        }
};

TEST(BlackBox, RTPSAsNonReliableSocket)
{
    RTPSAsNonReliableSocketReader reader;
    RTPSAsNonReliableSocketWriter writer;
    std::string ip("239.255.1.4");
    const uint32_t port = 22222;
    const uint16_t nmsgs = 100;
    
    reader.init(ip, port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(ip, port);

    if(!writer.isInitialized())
        return;

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(1));
    }

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    if(msgs.size() != 0)
    {
        std::cout << "Samples not received:";
        for(std::list<uint16_t>::iterator it = msgs.begin(); it != msgs.end(); ++it)
            std::cout << " " << *it << " ";
        std::cout << std::endl;
    }
    ASSERT_EQ(msgs.size(), 0);
}

TEST(BlackBox, RTPSAsReliableSocket)
{
    RTPSAsReliableSocketReader reader;
    RTPSAsReliableSocketWriter writer;
    std::string ip("239.255.1.4");
    const uint32_t port = 7400;
    const uint16_t nmsgs = 100;
    
    reader.init(ip, port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(ip, port);

    if(!writer.isInitialized())
        return;

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();

    writer.send(msgs);
    reader.block(*msgs.rbegin(), std::chrono::seconds(60));

    msgs = reader.getNonReceivedMessages();
    if(msgs.size() != 0)
    {
        std::cout << "Samples not received:";
        for(std::list<uint16_t>::iterator it = msgs.begin(); it != msgs.end(); ++it)
            std::cout << " " << *it << " ";
        std::cout << std::endl;
    }
    ASSERT_EQ(msgs.size(), 0);
}

TEST(BlackBox, RTPSAsNonReliableWithRegistration)
{
    RTPSAsNonReliableWithRegistrationReader reader;
    RTPSAsNonReliableWithRegistrationWriter writer;
    const uint32_t port = 22222;
    const uint16_t nmsgs = 100;
    
    reader.init(port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    if(!writer.isInitialized())
        return;

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(1));
    }

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    if(msgs.size() != 0)
    {
        std::cout << "Samples not received:";
        for(std::list<uint16_t>::iterator it = msgs.begin(); it != msgs.end(); ++it)
            std::cout << " " << *it << " ";
        std::cout << std::endl;
    }
    ASSERT_EQ(msgs.size(), 0);
}

TEST(BlackBox, RTPSAsReliableWithRegistration)
{
    RTPSAsReliableWithRegistrationReader reader;
    RTPSAsReliableWithRegistrationWriter writer;
    const uint32_t port = 7400;
    const uint16_t nmsgs = 100;
    
    reader.init(port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    if(!writer.isInitialized())
        return;

    reader.waitDiscovery();

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();

    writer.send(msgs);
    reader.block(*msgs.rbegin(), std::chrono::seconds(60));

    msgs = reader.getNonReceivedMessages();
    if(msgs.size() != 0)
    {
        std::cout << "Samples not received:";
        for(std::list<uint16_t>::iterator it = msgs.begin(); it != msgs.end(); ++it)
            std::cout << " " << *it << " ";
        std::cout << std::endl;
    }
    ASSERT_EQ(msgs.size(), 0);
}

TEST(BlackBox, PubSubAsNonReliableHelloworld)
{
    PubSubAsNonReliableHelloWorldReader reader;
    PubSubAsNonReliableHelloWorldWriter writer;
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    if(!writer.isInitialized())
        return;

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(1));
    }

    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    if(msgs.size() != 0)
    {
        std::cout << "Samples not received:";
        for(std::list<uint16_t>::iterator it = msgs.begin(); it != msgs.end(); ++it)
            std::cout << " " << *it << " ";
        std::cout << std::endl;
    }
    ASSERT_EQ(msgs.size(), 0);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);
    return RUN_ALL_TESTS();
}
