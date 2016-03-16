#include "types/HelloWorld.h"
#include "types/Data64kbType.h"
#include "types/Data1mbType.h"

#include "RTPSAsNonReliableSocketReader.hpp"
#include "RTPSAsNonReliableSocketWriter.hpp"
#include "RTPSAsReliableSocketReader.hpp"
#include "RTPSAsReliableSocketWriter.hpp"
#include "RTPSAsNonReliableWithRegistrationReader.hpp"
#include "RTPSAsNonReliableWithRegistrationWriter.hpp"
#include "RTPSAsReliableWithRegistrationReader.hpp"
#include "RTPSAsReliableWithRegistrationWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

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

/****** Auxiliary lambda functions  ******/
auto default_helloworld_receiver = [](eprosima::fastrtps::Subscriber* subscriber) -> uint16_t
{
    uint16_t ret = 0;
    HelloWorldType::type hello;
    SampleInfo_t info;

	if(subscriber->takeNextData((void*)&hello, &info))
	{
		if(info.sampleKind == ALIVE)
		{
            ret = hello.index();
		}
	}

    return ret;
};

auto default_helloworld_sender = [](eprosima::fastrtps::Publisher* publisher, const std::list<uint16_t>& msgs) -> void
{
	for(auto it = msgs.begin(); it != msgs.end(); ++it)
	{
        HelloWorldType::type hello;
        hello.index(*it);
        hello.message("HelloWorld");
        ASSERT_EQ(publisher->write((void*)&hello), true);
	}
};

auto default_data64kb_receiver = [](eprosima::fastrtps::Subscriber* subscriber) -> uint16_t
{
    uint16_t ret = 0;
    Data64kbType::type data;
    SampleInfo_t info;

    if(subscriber->takeNextData((void*)&data, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            ret = data.data()[0];
        }
    }

    return ret;
};

auto default_data64kb_sender = [](eprosima::fastrtps::Publisher* publisher, const std::list<uint16_t>& msgs) -> void
{
    Data64kbType::type data;
    for(int i = 0; i < 63996; ++i)
        data.data().push_back((unsigned char)i);

    for(auto it = msgs.begin(); it != msgs.end(); ++it)
    {
        data.data()[0] = (unsigned char)*it;
        ASSERT_EQ(publisher->write((void*)&data), true);
    }
};

auto default_data1mb_receiver = [](eprosima::fastrtps::Subscriber* subscriber) -> uint16_t
{
    uint16_t ret = 0;
    Data1mbType::type data;
    SampleInfo_t info;

    if(subscriber->takeNextData((void*)&data, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            ret = data.data()[0];
        }
    }

    return ret;
};

auto default_data1mb_sender = [](eprosima::fastrtps::Publisher* publisher, const std::list<uint16_t>& msgs) -> void
{
    Data1mbType::type data;
    for(int i = 0; i < 1024000; ++i)
        data.data().push_back((unsigned char)i);

    for(auto it = msgs.begin(); it != msgs.end(); ++it)
    {
        data.data()[0] = (unsigned char)*it;
        ASSERT_EQ(publisher->write((void*)&data), true);
    }
};
/***** End auxiliary lambda function *****/

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

    ASSERT_TRUE(writer.isInitialized());

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

TEST(BlackBox, AsyncRTPSAsNonReliableSocket)
{
    RTPSAsNonReliableSocketReader reader;
    RTPSAsNonReliableSocketWriter writer;
    std::string ip("239.255.1.4");
    const uint32_t port = 22222;
    const uint16_t nmsgs = 100;
    
    reader.init(ip, port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(ip, port, true);

    ASSERT_TRUE(writer.isInitialized());

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(2));
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

    ASSERT_TRUE(writer.isInitialized());

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

TEST(BlackBox, AsyncRTPSAsReliableSocket)
{
    RTPSAsReliableSocketReader reader;
    RTPSAsReliableSocketWriter writer;
    std::string ip("239.255.1.4");
    const uint32_t port = 7400;
    const uint16_t nmsgs = 100;
    
    reader.init(ip, port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(ip, port, true);

    ASSERT_TRUE(writer.isInitialized());

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

    ASSERT_TRUE(writer.isInitialized());

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

TEST(BlackBox, AsyncRTPSAsNonReliableWithRegistration)
{
    RTPSAsNonReliableWithRegistrationReader reader;
    RTPSAsNonReliableWithRegistrationWriter writer;
    const uint32_t port = 22222;
    const uint16_t nmsgs = 100;
    
    reader.init(port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(2));
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

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
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

TEST(BlackBox, AsyncRTPSAsReliableWithRegistration)
{
    RTPSAsReliableWithRegistrationReader reader;
    RTPSAsReliableWithRegistrationWriter writer;
    const uint32_t port = 7400;
    const uint16_t nmsgs = 100;
    
    reader.init(port, nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
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
    PubSubReader<HelloWorldType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> reader(default_helloworld_receiver);
    PubSubWriter<HelloWorldType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_helloworld_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

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

TEST(BlackBox, AsyncPubSubAsNonReliableHelloworld)
{
    PubSubReader<HelloWorldType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> reader(default_helloworld_receiver);
    PubSubWriter<HelloWorldType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_helloworld_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        reader.block(*msgs.rbegin(), std::chrono::seconds(2));
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

TEST(BlackBox, PubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> reader(default_helloworld_receiver);
    PubSubWriter<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> writer(default_helloworld_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
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

TEST(BlackBox, AsyncPubSubAsReliableHelloworld)
{
    PubSubReader<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> reader(default_helloworld_receiver);
    PubSubWriter<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> writer(default_helloworld_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
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

TEST(BlackBox, ReqRepAsReliableHelloworld)
{
    ReqRepAsReliableHelloWorldRequester requester;
    ReqRepAsReliableHelloWorldReplier replier;
    const uint16_t nmsgs = 100;

    requester.init();

    if(!requester.isInitialized())
        return;

    replier.init();

    if(!replier.isInitialized())
        return;

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        requester.send(count);
        requester.block(std::chrono::seconds(10));
    }
}

TEST(BlackBox, ParticipantRemoval)
{
    PubSubReader<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> reader(default_helloworld_receiver);
    PubSubWriter<HelloWorldType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> writer(default_helloworld_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability.
    reader.waitDiscovery();

    // Send some data.
    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    writer.send(msgs);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

TEST(BlackBox, PubSubAsReliableData64kb)
{
    PubSubReader<Data64kbType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> reader(default_data64kb_receiver);
    PubSubWriter<Data64kbType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> writer(default_data64kb_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability.
    reader.waitDiscovery();

    // Send some data.
    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    writer.send(msgs);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

TEST(BlackBox, AsyncPubSubAsReliableData64kb)
{
    PubSubReader<Data64kbType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> reader(default_data64kb_receiver);
    PubSubWriter<Data64kbType, eprosima::fastrtps::RELIABLE_RELIABILITY_QOS> writer(default_data64kb_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability.
    reader.waitDiscovery();

    // Send some data.
    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    writer.send(msgs);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

TEST(BlackBox, PubSubAsNonReliableData1mb)
{
    PubSubWriter<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_data1mb_sender);
    
    writer.init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST(BlackBox, AsyncPubSubAsNonReliableData1mb)
{
    PubSubReader<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> reader(default_data1mb_receiver);
    PubSubWriter<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_data1mb_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability.
    reader.waitDiscovery();

    // Send some data.
    std::list<uint16_t> msgs = reader.getNonReceivedMessages();
    writer.send(msgs);

    // Destroy the writer participant.
    writer.destroy();

    // Check that reader receives the unmatched.
    reader.waitRemoval();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);
#if defined(WIN32) && defined(_DEBUG)
    eprosima::Log::setVerbosity(eprosima::LOG_VERBOSITY_LVL::VERB_ERROR);
#endif
    return RUN_ALL_TESTS();
}
