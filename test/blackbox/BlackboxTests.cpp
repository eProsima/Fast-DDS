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

const size_t data64kb_length = 63996;
auto default_data64kb_receiver = [](eprosima::fastrtps::Subscriber* subscriber) -> uint16_t
{
    uint16_t ret = 0;
    Data64kbType::type data;
    SampleInfo_t info;

    if(subscriber->takeNextData((void*)&data, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            if(data.data().size() == data64kb_length)
            {
                size_t count = 1;
                for(;count < data64kb_length; ++count)
                {
                    if(data.data()[count] != (unsigned char)(count + data.data()[0]))
                       break; 
                }

                if(count == data64kb_length)
                {
                    ret = data.data()[0];
                }
                else
                    std::cout << "ERROR: received in position " << count << " the value " << static_cast<unsigned>(data.data()[count]) <<
                        " instead of " << static_cast<unsigned>((unsigned char)(count + data.data()[0])) << std::endl;
            }
        }
    }

    return ret;
};

auto default_data64kb_sender = [](eprosima::fastrtps::Publisher* publisher, const std::list<uint16_t>& msgs) -> void
{
    Data64kbType::type data;

    data.data().resize(data64kb_length);
    for(auto it = msgs.begin(); it != msgs.end(); ++it)
    {
        data.data()[0] = (unsigned char)*it;
        for(size_t i = 1; i < data64kb_length; ++i)
            data.data()[i] = (unsigned char)(i + data.data()[0]);

        ASSERT_EQ(publisher->write((void*)&data), true);
    }
};

const size_t data300kb_length = 307201;
auto default_data300kb_receiver = [](eprosima::fastrtps::Subscriber* subscriber) -> uint16_t
{
    uint16_t ret = 0;
    Data1mbType::type data;
    SampleInfo_t info;

    if(subscriber->takeNextData((void*)&data, &info))
    {
        if(info.sampleKind == ALIVE)
        {
            if(data.data().size() == data300kb_length)
            {
                size_t count = 1;
                for(;count < data300kb_length; ++count)
                {
                    if(data.data()[count] != (unsigned char)(count + data.data()[0]))
                       break; 
                }

                if(count == data300kb_length)
                {
                    ret = data.data()[0];
                }
                else
                    std::cout << "ERROR: received in position " << count << " the value " << static_cast<unsigned>(data.data()[count]) <<
                        " instead of " << static_cast<unsigned>((unsigned char)(count + data.data()[0])) << std::endl;
            }
        }
    }

    return ret;
};

auto default_data300kb_sender = [](eprosima::fastrtps::Publisher* publisher, const std::list<uint16_t>& msgs) -> void
{
    Data1mbType::type data;
    data.data().resize(data300kb_length);

    for(auto it = msgs.begin(); it != msgs.end(); ++it)
    {
        data.data()[0] = (unsigned char)*it;
        for(size_t i = 1; i < data300kb_length; ++i)
            data.data()[i] = (unsigned char)(i + data.data()[0]);

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

TEST(BlackBox, PubSubAsNonReliableData300kb)
{
    PubSubWriter<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_data300kb_sender);
    
    writer.init();

    ASSERT_FALSE(writer.isInitialized());
}

TEST(BlackBox, AsyncPubSubAsNonReliableData300kb)
{
    PubSubReader<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> reader(default_data300kb_receiver);
    PubSubWriter<Data1mbType, eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS> writer(default_data300kb_sender);
    const uint16_t nmsgs = 100;
    
    reader.init(nmsgs);

    if(!reader.isInitialized())
        return;

    writer.init(true);

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability.
    reader.waitDiscovery();

    for(unsigned int tries = 0; tries < 20; ++tries)
    {
        std::list<uint16_t> msgs = reader.getNonReceivedMessages();
        if(msgs.empty())
            break;

        writer.send(msgs);
        // Waiting, it needs more time than other tests because the fragments are large.
        reader.block(*msgs.rbegin(), std::chrono::seconds(10));
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
#if defined(WIN32) && defined(_DEBUG)
    eprosima::Log::setVerbosity(eprosima::LOG_VERBOSITY_LVL::VERB_ERROR);
#endif
    return RUN_ALL_TESTS();
}
