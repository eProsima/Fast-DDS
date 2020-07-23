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

#include "BlackboxTests.hpp"

#if HAVE_SQLITE3

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <cstring>
#include <thread>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::dds;

class PersistenceGuid : public ::testing::TestWithParam<bool>
{
protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        std::remove("persistence.db");
    }

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
    // Configure Writer Properties
    PropertyPolicy writer_policy;
    writer_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    writer_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    writer_policy.properties().emplace_back("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");

    // Create DataWriter and configure the durability and reliability QoS
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.entity_property_policy(writer_policy);
    writer.durability_kind(TRANSIENT_DURABILITY_QOS);
    writer.reliability(RELIABLE_RELIABILITY_QOS);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Configure Reader Properties
    PropertyPolicy reader_policy;
    reader_policy.properties().emplace_back("dds.persistence.plugin", "builtin.SQLITE3");
    reader_policy.properties().emplace_back("dds.persistence.sqlite3.filename", "persistence.db");
    reader_policy.properties().emplace_back("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");

    // Create DataReader and configure the durability and reliability QoS
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
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
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
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
    // Create DataWriter using XML Profile
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.set_xml_filename("persistence.xml");
    writer.set_participant_profile("persistence_participant");
    writer.set_datawriter_profile("persistence_data_writer");

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Create DataReader using XML Profile
    PubSubReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.set_xml_filename("persistence.xml");
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
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 1);
#endif // WIN32
}

INSTANTIATE_TEST_CASE_P(PersistenceGuid,
        PersistenceGuid,
        testing::Values(false, true),
        [](const testing::TestParamInfo<PersistenceGuid::ParamType>& info)
        {
            if (info.param)
            {
                return "Intraprocess";
            }
            return "NonIntraprocess";
        });
#endif // if HAVE_SQLITE3
