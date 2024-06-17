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
#include <iomanip>
#include <sstream>
#include <thread>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
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
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(att);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(att);
                break;
            case TRANSPORT:
            default:
                break;
        }
        std::remove(persistence_database().c_str());
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

    std::string persistence_database()
    {
        std::stringstream persistence_db;
        persistence_db << std::string("persistence_") << GET_PID() << std::string(".db");
        return persistence_db.str();
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
    std::string persistence_db = persistence_database();
    std::string guidprefix_4 = persistence_lastfour_guidprefix();
    std::stringstream guid;
    // Create RTPSWriter and configure the durability and property list
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.durability(TRANSIENT);
    writer.reliability(RELIABLE);
    writer.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    writer.add_property("dds.persistence.sqlite3.filename", persistence_db);
    guid << "77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64";
    writer.add_property("dds.persistence.guid", guid.str());
    guid.str("");

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Create RTPSReader and configure the durability and property list
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.durability(TRANSIENT);
    reader.reliability(RELIABLE);
    reader.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    reader.add_property("dds.persistence.sqlite3.filename", persistence_db);
    guid << "77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65";
    reader.add_property("dds.persistence.guid", guid.str());
    guid.str("");

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

    std::stringstream command;
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'readers' '77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
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
    std::string persistence_db = persistence_database();
    std::string guidprefix_4 = persistence_lastfour_guidprefix();
    std::stringstream guid;
    // Create the persistence guid to set manually
    eprosima::fastdds::rtps::GuidPrefix_t guidPrefix;
    int32_t pid = static_cast<int32_t>(GET_PID());
    guidPrefix.value[8] = reinterpret_cast<uint8_t*>(&pid)[0];
    guidPrefix.value[9] = reinterpret_cast<uint8_t*>(&pid)[1];
    guidPrefix.value[10] = reinterpret_cast<uint8_t*>(&pid)[2];
    guidPrefix.value[11] = reinterpret_cast<uint8_t*>(&pid)[3];
    eprosima::fastdds::rtps::EntityId_t entityId;
    entityId.value[3] = 1;

    // Create RTPSWriter that use the already created attributes
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    writer.durability(TRANSIENT);
    writer.reliability(RELIABLE);
    writer.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    writer.add_property("dds.persistence.sqlite3.filename", persistence_db);
    guid << "77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64";
    writer.add_property("dds.persistence.guid", guid.str());
    guid.str("");
    writer.persistence_guid_att(guidPrefix, entityId);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    entityId.value[3] = 2;

    // Create RTPSReader that use the already created attributes
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    reader.durability(TRANSIENT);
    reader.reliability(RELIABLE);
    reader.add_property("dds.persistence.plugin", "builtin.SQLITE3");
    reader.add_property("dds.persistence.sqlite3.filename", persistence_db);
    guid << "77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65";
    reader.add_property("dds.persistence.guid", guid.str());
    guid.str("");
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

    std::stringstream command;
#ifdef WIN32
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"writers_histories\" \"77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64\"";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 0);

    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"writers_histories\" \"00.00.00.00.00.00.00.00." << guidprefix_4 << "|0.0.0.1\"";
    result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result1, 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"readers\" \"77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65\"";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 0);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python check_guid.py \"" << persistence_db <<
        "\" \"readers\" \"00.00.00.00.00.00.00.00." << guidprefix_4 << "|0.0.0.2\"";
    result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ(result2, 1);
#else
    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'writers_histories' '77.72.69.74.65.72.5f.70." << guidprefix_4 << "|67.75.69.64'";
    int result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 0);

    // Check if there is one entry in the writers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'writers_histories' '00.00.00.00.00.00.00.00." << guidprefix_4 << "|0.0.0.1'";
    result1 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result1 >> 8), 1);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'readers' '77.65.61.64.65.72.5f.70." << guidprefix_4 << "|68.76.70.65'";
    int result2 = system(command.str().c_str());
    command.str("");
    ASSERT_EQ((result2 >> 8), 0);

    // Check if there is one entry in the readers database table with the stated persistence guid
    command << "python3 check_guid.py '" << persistence_db <<
        "' 'readers' '00.00.00.00.00.00.00.00." << guidprefix_4 << "|0.0.0.2'";
    result2 = system(command.str().c_str());
    command.str("");
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
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<PersistenceGuid::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }
        });
#endif // if HAVE_SQLITE3
