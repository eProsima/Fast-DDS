// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <climits>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds;

class HelloWorld
{
public:

    HelloWorld()
    {
    }

    ~HelloWorld()
    {
    }

    inline void message(const std::string &_message)
    {
        m_message = _message;
    }

    inline  const std::string& message() const
    {
        return m_message;
    }

    inline std::string& message()
    {
        return m_message;
    }

private:
    std::string m_message;
};

class TopicDataTypeMock : public TopicDataType
{
public:

    typedef HelloWorld type;

    TopicDataTypeMock()
        : TopicDataType()
    {
        setName("persistence_topic_type");
    }

    bool serialize(
            void* /*data*/,
            SerializedPayload_t* /*payload*/) override
    {
        return true;
    }

    bool deserialize(
            SerializedPayload_t* /*payload*/,
            void* /*data*/) override
    {
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(
            void* /*data*/) override
    {
        return std::function<uint32_t()>();
    }

    void* createData() override
    {
        return nullptr;
    }

    void deleteData(
            void* /*data*/) override
    {
    }

    bool getKey(
            void* /*data*/,
            InstanceHandle_t* /*ihandle*/,
            bool /*force_md5*/) override
    {
        return true;
    }

};


/*!
* @fn TEST(PersistenceGuidTest, DDS_set_writer_persistence_guid)
* @brief This test checks if the persistence guid is set correctly on the RTPSWriter
*/
TEST(PersistenceGuidTest, DDS_set_persistence_guid)
{
    HelloWorld hello;
    hello.message("HelloWorld");

    DomainParticipantQos pqos;
    pqos.properties().properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    pqos.properties().properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    ASSERT_NE(participant, nullptr);

    /* Register participant on type */
    TypeSupport type(new TopicDataTypeMock());
    type.register_type(participant);

    // Create a publisher and a subscriber
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    ASSERT_NE(publisher, nullptr);
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    ASSERT_NE(subscriber, nullptr);

    // Create a topic
    Topic* topic = participant->create_topic("persistence_topic_name", type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);

    // Configure DataWriter's durability and persistence GUID so it can use the persistence service
    DataWriterQos dwqos = DATAWRITER_QOS_DEFAULT;
    dwqos.durability().kind = TRANSIENT_DURABILITY_QOS;
    dwqos.properties().properties().emplace_back("dds.persistence.guid",
            "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");
    DataWriter* writer = publisher->create_datawriter(topic, dwqos);
    ASSERT_NE(writer, nullptr);

    // Configure DataReaders's durability and persistence GUID so it can use the persistence service
    DataReaderQos drqos = DATAREADER_QOS_DEFAULT;
    drqos.durability().kind = TRANSIENT_DURABILITY_QOS;
    drqos.properties().properties().emplace_back("dds.persistence.guid",
            "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");
    DataReader* reader = subscriber->create_datareader(topic, drqos);
    ASSERT_NE(reader, nullptr);

    writer->write(&hello);

    int result1 = system("python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    int result2 = system("python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 1);

    std::remove("persistence.db");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
