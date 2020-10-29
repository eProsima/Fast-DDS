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

#include <cstring>
#include <thread>

#include "RTPSWithRegistrationWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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
 * @fn TEST_P(PersistenceGuid, SetPersistenceGuidThroughRTPSLayer)
 * @brief This test checks if the persistence guid is set correctly on the RTPSWriter and RTPSReader when it is specified using the
 * property "dds.persistence.guid" through RTPS-layer API.
 *
 * This test creates a RTPSReader and RTPSWriter configuring for each of them the persistence plugin to use SQLITE3,
 * the database filename and a specific persistence guid.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware inserts
 * a new entry on the persistence database.
 * In order to check if the persistence guid specified is applied correctly, the test uses the python file 'check_guid.py'
 * that returns the number of apparitions of a guid, on a specific database and table.
 */
TEST_P(PersistenceGuid, SetPersistenceGuidThroughRTPSLayer)
{
    // Create RTPSWriter and configure the durability and property list
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.durability(TRANSIENT);
    writer.reliability(RELIABLE);
    writer.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    writer.add_property("dds.persistence.sqlite3.filename", "persistence.db");
    writer.add_property("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Create RTPSReader and configure the durability and property list
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.durability(TRANSIENT);
    reader.reliability(RELIABLE);
    reader.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    reader.add_property("dds.persistence.sqlite3.filename", "persistence.db");
    reader.add_property("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.expected_data(aux);
    reader.startReception(1);
    reader.block_for_all();

#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python check_guid.py \"persistence.db\" \"writers\" \"77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64\"");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python check_guid.py \"persistence.db\" \"readers\" \"77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65\"");
    ASSERT_EQ(result2, 1);
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
 * @fn TEST(PersistenceGuid, CheckPrevalenceBetweenManualAndPropertyConfiguration)
 * @brief This test checks which persistence guid prevails, the one set manually or the one set using the "dds.persistence.guid"
 * property.
 *
 * This test creates a RTPSReader and RTPSWriter configuring for each of them the persistence plugin to use SQLITE3,
 * the database filename. In this case, the persistence guid is set both manually and using the property.
 * Once both are discovered, it generates a new data message to send, since every time a new message is sent the middleware
 * inserts a new entry on the persistence database.
 * In order to check that the persistence guid that prevails among the two configured is the manual one, the test uses the python
 * file 'check_guid.py' that returns the number of apparitions of a guid, on a specific database and table.
 * It checks both the one set manually and the one set using the property, verifying that the manual one appears at least
 * once for readers and writers, and the property one doesn't appear.
 */
TEST_P(PersistenceGuid, CheckPrevalenceBetweenManualAndPropertyConfiguration)
{
    // Create the persistence guid to set manually
    eprosima::fastrtps::rtps::GuidPrefix_t guidPrefix;
    guidPrefix.value[11] = 1;
    eprosima::fastrtps::rtps::EntityId_t entityId;
    entityId.value[3] = 1;

    // Create RTPSWriter that use the already created attributes
    RTPSWithRegistrationWriter<HelloWorldType> writer(TEST_TOPIC_NAME);
    writer.durability(TRANSIENT);
    writer.reliability(RELIABLE);
    writer.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    writer.add_property("dds.persistence.sqlite3.filename", "persistence.db");
    writer.add_property("dds.persistence.guid", "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64");
    writer.persistence_guid_att(guidPrefix, entityId);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    guidPrefix.value[11] = 2;

    // Create RTPSReader that use the already created attributes
    RTPSWithRegistrationReader<HelloWorldType> reader(TEST_TOPIC_NAME);
    reader.durability(TRANSIENT);
    reader.reliability(RELIABLE);
    reader.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    reader.add_property("dds.persistence.sqlite3.filename", "persistence.db");
    reader.add_property("dds.persistence.guid", "77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65");
    reader.persistence_guid_att(guidPrefix, entityId);

    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait until both of them are discovered
    reader.wait_discovery();
    writer.wait_discovery();

    // Generate a new data message to send
    auto data = default_helloworld_data_generator(1);
    auto aux = data;
    writer.send(data);
    reader.expected_data(aux);
    reader.startReception(1);
    reader.block_for_all();

#ifdef WIN32
    // Check if that there is no entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python check_guid.py \"persistence.db\" \"writers\" \"77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64\"");
    ASSERT_EQ(result1, 0);

    // Check if there is one entry in the writers database table with the stated persistence guid
    result1 = system("python check_guid.py \"persistence.db\" \"writers\" \"0.0.0.0.0.0.0.0.0.0.0.1|0.0.0.1\"");
    ASSERT_EQ(result1, 1);

    // Check if that there is no entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python check_guid.py \"persistence.db\" \"readers\" \"77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65\"");
    ASSERT_EQ(result2, 0);

    // Check if there is one entry in the readers database table with the stated persistence guid
    result2 = system("python check_guid.py \"persistence.db\" \"readers\" \"0.0.0.0.0.0.0.0.0.0.0.2|0.0.0.1\"");
    ASSERT_EQ(result2, 1);
#else

    // Check if that there is no entry in the writers database table with the stated persistence guid
    int result1 = system(
        "python3 check_guid.py 'persistence.db' 'writers' '77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64'");
    ASSERT_EQ((result1 >> 8), 0);

    // Check if there is one entry in the writers database table with the stated persistence guid
    result1 = system("python3 check_guid.py 'persistence.db' 'writers' '0.0.0.0.0.0.0.0.0.0.0.1|0.0.0.1'");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if that there is no entry in the readers database table with the stated persistence guid
    int result2 = system(
        "python3 check_guid.py 'persistence.db' 'readers' '77.65.61.64.65.72.5f.70.65.72.73.5f|68.76.70.65'");
    ASSERT_EQ((result2 >> 8), 0);

    // Check if there is one entry in the readers database table with the stated persistence guid
    result2 = system("python3 check_guid.py 'persistence.db' 'readers' '0.0.0.0.0.0.0.0.0.0.0.2|0.0.0.1'");
    ASSERT_EQ((result2 >> 8), 1);
#endif // WIN32
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PersistenceGuid,
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
