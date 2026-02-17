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

#if HAVE_SQLITE3
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class PersistenceGuid : public ::testing::TestWithParam<communication_type>
{
protected:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery =
                        eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }

        // Get info about current test
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();

        // Create DB file name from test name and PID
        std::ostringstream ss;
        std::string test_case_name(info->test_case_name());
        std::string test_name(info->name());
        ss <<
            test_case_name.replace(test_case_name.find_first_of('/'), 1, "_") << "_" <<
            test_name.replace(test_name.find_first_of('/'), 1, "_")  << "_" << GET_PID() << ".db";
        db_file_name = ss.str();
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }

        std::remove(db_file_name.c_str());
    }

    std::string persistence_lastfour_guidprefix()
    {
        int32_t pid = static_cast<int32_t>(GET_PID());
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&pid);
        std::stringstream gp;
        gp << std::hex <<
            std::setfill('0') << std::setw(2) << static_cast<uint16_t>(bytes[0]) << "." <<
            std::setfill('0') << std::setw(2) << static_cast<uint16_t>(bytes[1]) << "." <<
            std::setfill('0') << std::setw(2) << static_cast<uint16_t>(bytes[2]) << "." <<
            std::setfill('0') << std::setw(2) << static_cast<uint16_t>(bytes[3]);
        return gp.str();
    }

    std::string persistence_xmlfile()
    {
        std::stringstream persistence_xml;
        persistence_xml << std::string("persistence_") << GET_PID() << std::string(".xml");
        return persistence_xml.str();
    }

    std::string db_file_name;
};

/*!
 * @fn TEST(PersistenceGuid, SetPersistenceGuidThroughDDSLayer)
 * @brief This test checks if the persistence guid is set correctly on the DataWriter and DataReader when it is specified using
 * the property "dds.persistence.guid" through DDS-layer API.
 *
 * This test creates a DataReader and DataWriter configuring for each of them the persistence plugin to use SQLITE3,
 * the database filename, a specific persistence guid, and the durability and reliability QoS.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware inserts
 * a new entry on the persistence database.
 * In order to check if the persistence guid specified is applied correctly, the test uses the python file 'check_guid.py'
 * that returns the number of apparitions of a guid, on a specific database and table.
 */
TEST_P(PersistenceGuid, SetPersistenceGuidThroughDDSLayer)
{
    std::string guidprefix_4 = persistence_lastfour_guidprefix();
    std::stringstream guid;
    // Configure Writer Properties
    PropertyPolicy writer_policy;
    writer_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    writer_policy.properties().emplace_back("dds.persistence.sqlite3.filename", db_file_name);
    guid << "77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64";
    writer_policy.properties().emplace_back("dds.persistence.guid", guid.str());
    guid.str("");

    // Create DataWriter and configure the durability and reliability QoS
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.entity_property_policy(writer_policy);
    writer.durability_kind(TRANSIENT_DURABILITY_QOS);
    writer.reliability(RELIABLE_RELIABILITY_QOS);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Configure Reader Properties
    PropertyPolicy reader_policy;
    reader_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    reader_policy.properties().emplace_back("dds.persistence.sqlite3.filename", db_file_name);
    guid << "77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65";
    reader_policy.properties().emplace_back("dds.persistence.guid", guid.str());
    guid.str("");

    // Create DataReader and configure the durability and reliability QoS
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.entity_property_policy(reader_policy);
    reader.durability_kind(TRANSIENT_DURABILITY_QOS);
    reader.reliability(RELIABLE_RELIABILITY_QOS);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.startReception(aux);
    reader.block_for_all();

    std::stringstream command;
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'readers' '77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result2 >> 8), 1);
#endif // WIN32

}


/*!
 * @fn TEST(PersistenceGuid, SetPersistenceGuidByXML)
 * @brief This test checks if the persistence guid is set correctly on the DataWriter and DataReader when it is specified using
 * the property "dds.persistence.guid" through an XML file.
 *
 * This test creates a DataReader and DataWriter using the XML file persitence.xml, which configures for each of them the persistence
 * plugin to use SQLITE3, the database filename, a specific persistence guid, and several QoS.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware inserts
 * a new entry on the persistence database.
 * In order to check if the persistence guid specified is applied correctly, the test uses the python file 'check_guid.py'
 * that returns the number of apparitions of a guid, on a specific database and table.
 */
TEST_P(PersistenceGuid, SetPersistenceGuidByXML)
{
    static std::string xml =
            "                                                                                     \
                <?xml version=\"1.0\" encoding=\"utf-8\"  ?>                                      \
                <dds>                                                                             \
                    <profiles>                                                                    \
                        <participant profile_name=\"persistence_participant\">                    \
                            <rtps>                                                                \
                                <propertiesPolicy>                                                \
                                    <properties>                                                  \
                                        <property>                                                \
                                            <name>dds.persistence.plugin</name>                   \
                                            <value>builtin.SQLITE3</value>                        \
                                        </property>                                               \
                                        <property>                                                \
                                            <name>dds.persistence.sqlite3.filename</name>         \
                                            <value>DATABASE</value>                               \
                                        </property>                                               \
                                    </properties>                                                 \
                                </propertiesPolicy>                                               \
                            </rtps>                                                               \
                        </participant>                                                            \
                                                                                                  \
                        <data_writer profile_name=\"persistence_data_writer\">                    \
                            <qos>                                                                 \
                                <durability>                                                      \
                                    <kind>TRANSIENT</kind>                                        \
                                </durability>                                                     \
                                <reliability>                                                     \
                                    <kind>RELIABLE</kind>                                         \
                                    <max_blocking_time>                                           \
                                        <sec>1</sec>                                              \
                                        <nanosec>0</nanosec>                                      \
                                    </max_blocking_time>                                          \
                                </reliability>                                                    \
                            </qos>                                                                \
                            <times>                                                               \
                                <heartbeat_period>                                                 \
                                    <sec>0</sec>                                                  \
                                    <nanosec>100000000</nanosec>                                  \
                                </heartbeat_period>                                                \
                                <nack_response_delay>                                               \
                                    <sec>0</sec>                                                  \
                                    <nanosec>100000000</nanosec>                                  \
                                </nack_response_delay>                                              \
                            </times>                                                              \
                            <historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>               \
                            <propertiesPolicy>                                                    \
                                <properties>                                                      \
                                    <property>                                                    \
                                        <name>dds.persistence.guid</name>                         \
                                        <value>77.72.69.74.65.72.5f.70.GID|67.75.69.64</value>    \
                                    </property>                                                   \
                                </properties>                                                     \
                            </propertiesPolicy>                                                   \
                        </data_writer>                                                            \
                        <data_reader profile_name=\"persistence_data_reader\">                    \
                            <qos>                                                                 \
                                <durability>                                                      \
                                    <kind>TRANSIENT</kind>                                        \
                                </durability>                                                     \
                                <reliability>                                                     \
                                    <kind>RELIABLE</kind>                                         \
                                </reliability>                                                    \
                            </qos>                                                                \
                            <historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>               \
                            <times>                                                               \
                                <heartbeat_response_delay>                                          \
                                    <sec>0</sec>                                                  \
                                    <nanosec>100000000</nanosec>                                  \
                                </heartbeat_response_delay>                                         \
                            </times>                                                              \
                            <propertiesPolicy>                                                    \
                                <properties>                                                      \
                                    <property>                                                    \
                                        <name>dds.persistence.guid</name>                         \
                                        <value>77.65.61.64.65.72.5f.70.GID|68.76.70.65</value>    \
                                    </property>                                                   \
                                </properties>                                                     \
                            </propertiesPolicy>                                                   \
                        </data_reader>                                                            \
                    </profiles>                                                                   \
                </dds>                                                                            \
        ";
    std::string guidprefix_4 = persistence_lastfour_guidprefix();
    std::string persistence_xml = persistence_xmlfile();

    const std::string database_tag = "DATABASE";
    xml = xml.replace(xml.find(database_tag), database_tag.length(), db_file_name);
    const std::string gid_tag = "GID";
    xml = xml.replace(xml.find(gid_tag), gid_tag.length(), guidprefix_4);
    xml = xml.replace(xml.find(gid_tag), gid_tag.length(), guidprefix_4);

    std::wofstream xml_out{persistence_xml};
    xml_out << xml.c_str() << std::endl;
    xml_out.close();

    class ScopeExit
    {
    public:

        ScopeExit(
                std::function<void()> f)
            : f_(f)
        {
        }

        ~ScopeExit()
        {
            f_();
        }

    private:

        std::function<void()> f_;
    }
    scope_exit([&persistence_xml]()
            {
                std::remove(persistence_xml.c_str());
            }
            );

    // Create DataWriter using XML Profile
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.set_xml_filename(persistence_xml);
    writer.set_participant_profile("persistence_participant");
    writer.set_datawriter_profile("persistence_data_writer");

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Create DataReader using XML Profile
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.set_xml_filename(persistence_xml);
    reader.set_participant_profile("persistence_participant");
    reader.set_datareader_profile("persistence_data_reader");

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.startReception(aux);
    reader.block_for_all();

    std::stringstream command;
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 1);
#else
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'readers' '77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result2 >> 8), 1);
#endif // WIN32
}

/*!
 * @fn TEST(Persistence, SetPersistenceForTransientLocal)
 * @brief This test checks if the persistence works for TRANSIENT_LOCAL endpoints when it is used the option
 * 'dds.persistence.also-support-transient-local'.
 *
 * This test creates a DataReader and DataWriter configuring for each of them the persistence plugin to use SQLITE3,
 * the database filename, a specific persistence guid, and the TRANSIENT_LOCAL durability and reliability QoS.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware inserts
 * a new entry on the persistence database.
 * In order to check if the persistence guid specified is applied correctly, the test uses the python file 'check_guid.py'
 * that returns the number of apparitions of a guid, on a specific database and table.
 */
TEST_P(PersistenceGuid, SetPersistenceForTransientLocal)
{
    std::string guidprefix_4 = persistence_lastfour_guidprefix();
    std::stringstream guid;
    // Configure Participant Properties
    PropertyPolicy participant_policy;
    participant_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    participant_policy.properties().emplace_back("dds.persistence.sqlite3.filename", db_file_name);
    participant_policy.properties().emplace_back("dds.persistence.also-support-transient-local", "true");

    // Configure Writer Properties
    PropertyPolicy writer_policy;
    guid << "77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64";
    writer_policy.properties().emplace_back("dds.persistence.guid", guid.str());
    guid.str("");

    // Create DataWriter and configure the durability and reliability QoS
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.property_policy(participant_policy);
    writer.entity_property_policy(writer_policy);
    writer.durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS);
    writer.reliability(RELIABLE_RELIABILITY_QOS);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Configure Reader Properties
    PropertyPolicy reader_policy;
    guid << "77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65";
    reader_policy.properties().emplace_back("dds.persistence.guid", guid.str());
    guid.str("");

    // Create DataReader and configure the durability and reliability QoS
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.property_policy(participant_policy);
    reader.entity_property_policy(reader_policy);
    reader.durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS);
    reader.reliability(RELIABLE_RELIABILITY_QOS);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.startReception(aux);
    reader.block_for_all();
    std::stringstream command;
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << db_file_name <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'readers' '77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result2 >> 8), 1);
#endif // WIN32

}

/*!
 * @fn TEST(Persistence, NoSetPersistenceForTransientLocal)
 * @brief This test checks if the persistence desn't work for TRANSIENT_LOCAL endpoints when it is not used the option
 * 'dds.persistence.also-support-transient-local'.
 *
 * This test creates a DataReader and DataWriter configuring for each of them the persistence plugin to use SQLITE3,
 * the database filename, a specific persistence guid, and the TRANSIENT_LOCAL durability and reliability QoS.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware doesn't insert
 * a new entry on the persistence database.
 * In order to check if the persistence guid specified is not applied correctly, the test uses the python file 'check_guid.py'
 * that returns the number of apparitions of a guid, on a specific database and table.
 */
TEST_P(PersistenceGuid, NoSetPersistenceForTransientLocal)
{
    // Configure Participant Properties
    PropertyPolicy participant_policy;
    participant_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    participant_policy.properties().emplace_back("dds.persistence.sqlite3.filename", db_file_name);

    // Configure Writer Properties
    PropertyPolicy writer_policy;
    writer_policy.properties().emplace_back("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");

    // Create DataWriter and configure the durability and reliability QoS
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.property_policy(participant_policy);
    writer.entity_property_policy(writer_policy);
    writer.durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS);
    writer.reliability(RELIABLE_RELIABILITY_QOS);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Configure Reader Properties
    PropertyPolicy reader_policy;
    reader_policy.properties().emplace_back("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");

    // Create DataReader and configure the durability and reliability QoS
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.property_policy(participant_policy);
    reader.entity_property_policy(reader_policy);
    reader.durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS);
    reader.reliability(RELIABLE_RELIABILITY_QOS);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.startReception(aux);
    reader.block_for_all();
    std::stringstream command;
#ifdef WIN32
    // Check if there is no entry in the writers database table with the stated persistence guid
    command <<  "python check_guid.py \"" << db_file_name <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 255);

    // Check if there is no entry in the readers database table with the stated persistence guid
    command <<  "python check_guid.py \"" << db_file_name <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 255);
#else
    // Check if there is no entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 255);

    // Check if there is no entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << db_file_name <<
        "' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result2 >> 8), 255);
#endif // WIN32

}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PersistenceGuid,
        PersistenceGuid,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<PersistenceGuid::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
#endif // if HAVE_SQLITE3
