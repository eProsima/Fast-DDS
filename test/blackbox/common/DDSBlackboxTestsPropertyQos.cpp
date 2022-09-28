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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"

#include "PubSubParticipant.hpp"

#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/attributes/LibrarySettingsAttributes.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace test {
const std::string EXTERNAL_PROPERTY_NAME  = "CustomExternalProperty";
const std::string EXTERNAL_PROPERTY_VALUE = "My Value";
const std::string INTERNAL_PROPERTY_NAME  = "CustomInternalProperty";
const std::string INTERNAL_PROPERTY_VALUE = "Other Value";
} // namespace test


using namespace eprosima::fastrtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class PropertyQos : public testing::TestWithParam<communication_type>
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
 * This test checks that the property qos are correctly sent once the participant is initialized.
 * In order to check that the properties are correctly updated, two participants are created and the discovery info is
 * checked. It should contain those properties that were meant to be sent, and not those that did not.
 */
TEST_P(PropertyQos, send_property_qos)
{
    // Set Properties that will be sent and those that wont
    eprosima::fastrtps::rtps::PropertyPolicy source_property_policy;

    // Add external property
    {
        eprosima::fastrtps::rtps::Property property;
        property.name(test::EXTERNAL_PROPERTY_NAME);
        property.value(test::EXTERNAL_PROPERTY_VALUE);
        property.propagate(true);
        source_property_policy.properties().push_back(property);
    }
    // Add internal property
    {
        eprosima::fastrtps::rtps::Property property;
        property.name(test::INTERNAL_PROPERTY_NAME);
        property.value(test::INTERNAL_PROPERTY_VALUE);
        property.propagate(false);
        source_property_policy.properties().push_back(property);
    }

    PubSubParticipant<HelloWorldPubSubType> participant_1(0u, 0u, 0u, 0u);
    participant_1.property_policy(source_property_policy);
    ASSERT_TRUE(participant_1.init_participant());

    PubSubParticipant<HelloWorldPubSubType> participant_2(0u, 0u, 0u, 0u);

    participant_2.set_on_discovery_function([&](const rtps::ParticipantDiscoveryInfo& info) -> bool
            {
                std::cout << "Received Property Qos: ";

                // Check the external has arrived and the internal does not
                bool property_received = false;
                for (auto i : info.info.m_properties)
                {
                    std::cout << i.first() << " :{ " << i.second() << " } ; ";

                    // Check the internal is not received
                    if (test::INTERNAL_PROPERTY_NAME == i.first())
                    {
                        return false;
                    }

                    // If it is the external, check the value is correct
                    if (test::EXTERNAL_PROPERTY_NAME == i.first())
                    {
                        // Avoid double property
                        if (property_received)
                        {
                            return false;
                        }
                        else
                        {
                            property_received = true;
                        }

                        if (test::EXTERNAL_PROPERTY_VALUE != i.second())
                        {
                            return false;
                        }
                    }
                }
                std::cout << std::endl;
                return property_received;
            });

    ASSERT_TRUE(participant_2.init_participant());

    participant_1.wait_discovery();
    participant_2.wait_discovery_result();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PropertyQos,
        PropertyQos,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<PropertyQos::ParamType>& info)
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
