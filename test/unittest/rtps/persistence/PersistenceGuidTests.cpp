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

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/writer/RTPSWriter.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <climits>
#include <tinyxml2.h>
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
* @fn TEST(PersistenceGuidTest, DDS_persistence_guid)
* @brief This test checks if the persistence guid is set correctly on the RTPSWriter when it is specified using the property
* dds.persistence.guid through DDS layer
*/
TEST(PersistenceGuidTest, DDS_persistence_guid)
{
    HelloWorld hello;
    hello.message("HelloWorld");

    // Create participant
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

/*!
* @fn TEST(PersistenceGuidTest, RTPS_persistence_guid)
* @brief This test checks if the persistence guid is set correctly on the RTPSWriter when it is specified using the property
* dds.persistence.guid through RTPS layer
*/
TEST(PersistenceGuidTest, RTPS_persistence_guid)
{
    // Create participant
    RTPSParticipantAttributes PParam;
    PParam.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.builtin.use_WriterLivelinessProtocol = true;
    RTPSParticipant* participant = RTPSDomain::createParticipant(0, PParam);
    ASSERT_NE(participant, nullptr);

    // Create WriterHistory
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    hatt.maximumReservedCaches = 50;
    WriterHistory* w_history = new WriterHistory(hatt);
    ASSERT_NE(w_history, nullptr);

    PropertyPolicy writer_policy;
    writer_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    writer_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    writer_policy.properties().emplace_back("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");

    //CREATE WRITER
    WriterAttributes watt;
    watt.endpoint.durabilityKind = TRANSIENT;
    watt.endpoint.properties = writer_policy;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, watt, w_history, nullptr);
    ASSERT_NE(writer, nullptr);

    eprosima::fastrtps::TopicAttributes Tatt;
    Tatt.topicKind = NO_KEY;
    Tatt.topicDataType = "string";
    Tatt.topicName = "exampleTopic";
    WriterQos Wqos;
    participant->registerWriter(writer, Tatt, Wqos);

    // Create ReaderHistory
    HistoryAttributes hatt1;
    hatt1.payloadMaxSize = 255;
    ReaderHistory* r_history = new ReaderHistory(hatt1);

    PropertyPolicy reader_policy;
    reader_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    reader_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    reader_policy.properties().emplace_back("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");

    // Create Reader
    ReaderAttributes ratt;
    Locator_t loc(22222);
    ratt.endpoint.unicastLocatorList.push_back(loc);
    ratt.endpoint.durabilityKind = TRANSIENT;
    ratt.endpoint.properties = reader_policy;
    RTPSReader* reader = RTPSDomain::createRTPSReader(participant, ratt, r_history, nullptr);
    ASSERT_NE(reader, nullptr);

    eprosima::fastrtps::TopicAttributes Tatt_r;
    Tatt_r.topicKind = NO_KEY;
    Tatt_r.topicDataType = "string";
    Tatt_r.topicName = "exampleTopic";
    ReaderQos Rqos;
    participant->registerReader(reader, Tatt_r, Rqos);

    CacheChange_t* ch = writer->new_change([]() -> uint32_t {
            return 255;
        }, ALIVE);

#if defined(_WIN32)
    ch->serializedPayload.length =
            sprintf_s((char*)ch->serializedPayload.data, 255, "My example string") + 1;
#else
    ch->serializedPayload.length =
            sprintf((char*)ch->serializedPayload.data, "My example string") + 1;
#endif
    w_history->add_change(ch);

    int result1 = system("python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    int result2 = system("python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 1);

    std::remove("persistence.db");
}

/*!
* @fn TEST(PersistenceGuidTest, check_prevalence)
* @brief This test checks which persistence guid prevails the one set manually or the one set using the dds.persistence.guid
* property
*/
TEST(PersistenceGuidTest, check_prevalence)
{
    // Create participant
    RTPSParticipantAttributes PParam;
    PParam.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.builtin.use_WriterLivelinessProtocol = true;
    RTPSParticipant* participant = RTPSDomain::createParticipant(0, PParam);
    ASSERT_NE(participant, nullptr);

    // Create WriterHistory
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 255;
    hatt.maximumReservedCaches = 50;
    WriterHistory* w_history = new WriterHistory(hatt);
    ASSERT_NE(w_history, nullptr);

    PropertyPolicy writer_policy;
    writer_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    writer_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    writer_policy.properties().emplace_back("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");

    //CREATE WRITER
    WriterAttributes watt;
    watt.endpoint.durabilityKind = TRANSIENT;
    watt.endpoint.properties = writer_policy;
    watt.endpoint.persistence_guid.guidPrefix.value[11] = 1;
    watt.endpoint.persistence_guid.entityId.value[3] = 1;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, watt, w_history, nullptr);
    ASSERT_NE(writer, nullptr);

    eprosima::fastrtps::TopicAttributes Tatt;
    Tatt.topicKind = NO_KEY;
    Tatt.topicDataType = "string";
    Tatt.topicName = "exampleTopic";
    WriterQos Wqos;
    participant->registerWriter(writer, Tatt, Wqos);

    // Create ReaderHistory
    HistoryAttributes hatt1;
    hatt1.payloadMaxSize = 255;
    ReaderHistory* r_history = new ReaderHistory(hatt1);

    PropertyPolicy reader_policy;
    reader_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    reader_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    reader_policy.properties().emplace_back("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");

    // Create Reader
    ReaderAttributes ratt;
    Locator_t loc(22222);
    ratt.endpoint.unicastLocatorList.push_back(loc);
    ratt.endpoint.durabilityKind = TRANSIENT;
    ratt.endpoint.properties = reader_policy;
    ratt.endpoint.persistence_guid.guidPrefix.value[11] = 2;
    ratt.endpoint.persistence_guid.entityId.value[3] = 1;
    RTPSReader* reader = RTPSDomain::createRTPSReader(participant, ratt, r_history, nullptr);
    ASSERT_NE(reader, nullptr);

    eprosima::fastrtps::TopicAttributes Tatt_r;
    Tatt_r.topicKind = NO_KEY;
    Tatt_r.topicDataType = "string";
    Tatt_r.topicName = "exampleTopic";
    ReaderQos Rqos;
    participant->registerReader(reader, Tatt_r, Rqos);

    CacheChange_t* ch = writer->new_change([]() -> uint32_t {
            return 255;
        }, ALIVE);

#if defined(_WIN32)
    ch->serializedPayload.length =
            sprintf_s((char*)ch->serializedPayload.data, 255, "My example string") + 1;
#else
    ch->serializedPayload.length =
            sprintf((char*)ch->serializedPayload.data, "My example string") + 1;
#endif
    w_history->add_change(ch);

    int result1 = system("python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 0);

    result1 = system("python3 check_guid.py 'persistence.db' 'writers' '0.0.0.0.0.0.0.0.0.0.0.1|0.0.0.1'");
    ASSERT_EQ((result1 >> 8), 1);

    int result2 = system("python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 0);

    result2 = system("python3 check_guid.py 'persistence.db' 'readers' '0.0.0.0.0.0.0.0.0.0.0.2|0.0.0.1'");
    ASSERT_EQ((result2 >> 8), 1);

    std::remove("persistence.db");
}

/*!
* @fn TEST(PersistenceGuidTest, persistence_guid_by_xml)
* @brief This test checks if the persistence guid is set correctly on the RTPSWriter when it is specified using an XML
*/
TEST(PersistenceGuidTest, persistence_guid_by_xml)
{
    HelloWorld hello;
    hello.message("HelloWorld");

    // Create participant
    DomainParticipantFactory::get_instance()->load_XML_profiles_file("persistence.xml");
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->
            create_participant_with_profile(0, "persistence_participant");
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
    DataWriter* writer = publisher->create_datawriter_with_profile(topic, "persistence_data_writer");
    ASSERT_NE(writer, nullptr);

    // Configure DataReaders's durability and persistence GUID so it can use the persistence service
    DataReader* reader = subscriber->create_datareader_with_profile(topic, "persistence_data_reader");
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
