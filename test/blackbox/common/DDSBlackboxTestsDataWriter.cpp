// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <gtest/gtest.h>

#include <fastrtps/attributes/LibrarySettingsAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <rtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;
using test_UDPv4TransportDescriptor = eprosima::fastdds::rtps::test_UDPv4TransportDescriptor;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class DDSDataWriter : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

/**
 * Test that checks DataWriter::wait_for_acknowledgments for a specific instance
 */
TEST_P(DDSDataWriter, WaitForAcknowledgmentInstance)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();

    writer.disable_builtin_transport().add_user_transport_to_pparams(testTransport).init();
    ASSERT_TRUE(writer.isInitialized());

    reader.reliability(RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Disable communication to prevent reception of ACKs
    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, rtps::c_InstanceHandle_Unknown);
    auto instance_handle_2 = writer.register_instance(data.back());
    EXPECT_NE(instance_handle_2, rtps::c_InstanceHandle_Unknown);

    reader.startReception(data);

    writer.send(data);
    EXPECT_TRUE(data.empty());

    // Intraprocess does not use transport layer. The ACKs cannot be disabled.
    if (INTRAPROCESS != GetParam())
    {
        EXPECT_FALSE(writer.waitForInstanceAcked(instance_handle_1, std::chrono::seconds(1)));
        EXPECT_FALSE(writer.waitForInstanceAcked(instance_handle_2, std::chrono::seconds(1)));
    }

    // Enable communication and wait for acknowledgment
    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

    EXPECT_TRUE(writer.waitForInstanceAcked(instance_handle_1, std::chrono::seconds(1)));
    EXPECT_TRUE(writer.waitForInstanceAcked(instance_handle_2, std::chrono::seconds(1)));

}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSDataWriter,
        DDSDataWriter,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<DDSDataWriter::ParamType>& info)
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
