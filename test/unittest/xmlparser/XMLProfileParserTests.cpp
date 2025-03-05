// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <thread>

#include <gtest/gtest.h>
#include <tinyxml2.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/network/AllowedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/BlockedNetworkInterface.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPTransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <xmlparser/XMLProfileManager.h>

#include "../common/env_var_utils.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::testing;
using namespace ::testing;

using eprosima::fastdds::dds::Log;
using eprosima::fastdds::dds::LogMock;
using eprosima::fastdds::dds::LogConsumer;

LogMock* log_mock = nullptr;

// Initialize Log mock
void TestRegisterConsumerFunc(
        std::unique_ptr<LogConsumer>&& c)
{
    log_mock->RegisterConsumer(std::move(c));
}

void TestClearConsumersFunc()
{
    log_mock->ClearConsumers();
}

void TestSetThreadConfigFunc()
{
    log_mock->SetThreadConfig();
}

std::function<void(std::unique_ptr<LogConsumer>&&)> Log::RegisterConsumerFunc = TestRegisterConsumerFunc;
std::function<void()> Log::ClearConsumersFunc = TestClearConsumersFunc;
std::function<void()> Log::SetThreadConfigFunc = TestSetThreadConfigFunc;

class XMLProfileParserBasicTests : public ::testing::Test
{
public:

    XMLProfileParserBasicTests()
    {
        log_mock = new testing::StrictMock<LogMock>();
    }

    ~XMLProfileParserBasicTests()
    {
        delete log_mock;
    }

protected:

    void SetUp() override
    {
        // Unload the profiles that might have been loaded by a previous test
        xmlparser::XMLProfileManager::DeleteInstance();
    }

};

class XMLProfileParserTests : public XMLProfileParserBasicTests, public testing::WithParamInterface<bool>
{
protected:

    void SetUp() override
    {
        XMLProfileParserBasicTests::SetUp();

        // Use plain file by default
        xml_filename_ = "test_xml_profile.xml";

        // Check if loading from environment variables should be tested
        if (GetParam())
        {
            // Use different file
            xml_filename_ = "test_xml_profile_env_var.xml";

            // Set environment variables values
            for (const std::pair<std::string, std::string>& value : c_environment_values_)
            {
                set_environment_variable(value.first.c_str(), value.second.c_str());
            }
        }
    }

    void TearDown() override
    {
        if (GetParam())
        {
            for (const std::pair<std::string, std::string>& value : c_environment_values_)
            {
                clear_environment_variable(value.first.c_str());
            }
        }
    }

    std::string xml_filename_ = "test_xml_profile.xml";

    const std::pair<std::string, std::string> c_environment_values_[169]
    {
        {"XML_PROFILES_ENV_VAR_1",   "123"},
        {"XML_PROFILES_ENV_VAR_2",   "4"},
        {"XML_PROFILES_ENV_VAR_3",   "1"},
        {"XML_PROFILES_ENV_VAR_4",   "10"},
        {"XML_PROFILES_ENV_VAR_5",   "20"},
        {"XML_PROFILES_ENV_VAR_6",   "2"},
        {"XML_PROFILES_ENV_VAR_7",   "10"},
        {"XML_PROFILES_ENV_VAR_8",   "20"},
        {"XML_PROFILES_ENV_VAR_9",   "2"},
        {"XML_PROFILES_ENV_VAR_10",  "10"},
        {"XML_PROFILES_ENV_VAR_11",  "20"},
        {"XML_PROFILES_ENV_VAR_12",  "2"},
        {"XML_PROFILES_ENV_VAR_13",  "127"},
        {"XML_PROFILES_ENV_VAR_14",  "true"},
        {"XML_PROFILES_ENV_VAR_15",  "192.168.1.2"},
        {"XML_PROFILES_ENV_VAR_16",  "2019"},
        {"XML_PROFILES_ENV_VAR_17",  "239.255.0.1"},
        {"XML_PROFILES_ENV_VAR_18",  "2021"},
        {"XML_PROFILES_ENV_VAR_19",  "true"},
        {"XML_PROFILES_ENV_VAR_20",  "10.10.10.10"},
        {"XML_PROFILES_ENV_VAR_21",  "2001"},
        {"XML_PROFILES_ENV_VAR_22",  "32"},
        {"XML_PROFILES_ENV_VAR_23",  "1000"},
        {"XML_PROFILES_ENV_VAR_24",  "SIMPLE"},
        {"XML_PROFILES_ENV_VAR_25",  "SIMPLE"},
        {"XML_PROFILES_ENV_VAR_26",  "FILTER_SAME_PROCESS | FILTER_DIFFERENT_HOST"},
        {"XML_PROFILES_ENV_VAR_27",  "10"},
        {"XML_PROFILES_ENV_VAR_28",  "333"},
        {"XML_PROFILES_ENV_VAR_29",  "DURATION_INFINITY"},
        {"XML_PROFILES_ENV_VAR_30",  "2"},
        {"XML_PROFILES_ENV_VAR_31",  "1"},
        {"XML_PROFILES_ENV_VAR_32",  "827"},
        {"XML_PROFILES_ENV_VAR_33",  "false"},
        {"XML_PROFILES_ENV_VAR_34",  "true"},
        {"XML_PROFILES_ENV_VAR_35",  "false"},
        {"XML_PROFILES_ENV_VAR_36",  "false"},
        {"XML_PROFILES_ENV_VAR_37",  "192.168.1.5"},
        {"XML_PROFILES_ENV_VAR_38",  "9999"},
        {"XML_PROFILES_ENV_VAR_39",  "192.168.1.6"},
        {"XML_PROFILES_ENV_VAR_40",  "6666"},
        {"XML_PROFILES_ENV_VAR_41",  "239.255.0.2"},
        {"XML_PROFILES_ENV_VAR_42",  "32"},
        {"XML_PROFILES_ENV_VAR_43",  "239.255.0.3"},
        {"XML_PROFILES_ENV_VAR_44",  "2112"},
        {"XML_PROFILES_ENV_VAR_45",  "10.10.10.10"},
        {"XML_PROFILES_ENV_VAR_46",  "2002"},
        {"XML_PROFILES_ENV_VAR_47",  "239.255.0.1"},
        {"XML_PROFILES_ENV_VAR_48",  "21120"},
        {"XML_PROFILES_ENV_VAR_49",  "PREALLOCATED"},
        {"XML_PROFILES_ENV_VAR_50",  "PREALLOCATED"},
        {"XML_PROFILES_ENV_VAR_51",  "1000"},
        {"XML_PROFILES_ENV_VAR_52",  "2000"},
        {"XML_PROFILES_ENV_VAR_53",  "55"},
        {"XML_PROFILES_ENV_VAR_54",  "true"},
        {"XML_PROFILES_ENV_VAR_55",  "true"},
        {"XML_PROFILES_ENV_VAR_56",  "12"},
        {"XML_PROFILES_ENV_VAR_57",  "34"},
        {"XML_PROFILES_ENV_VAR_58",  "56"},
        {"XML_PROFILES_ENV_VAR_59",  "78"},
        {"XML_PROFILES_ENV_VAR_60",  "90"},
        {"XML_PROFILES_ENV_VAR_61",  "123"},
        {"XML_PROFILES_ENV_VAR_62",  "456"},
        {"XML_PROFILES_ENV_VAR_63",  "9898"},
        {"XML_PROFILES_ENV_VAR_64",  "true"},
        {"XML_PROFILES_ENV_VAR_65",  "test_name"},
        {"XML_PROFILES_ENV_VAR_66",  "56.30.0.ce"},
        {"XML_PROFILES_ENV_VAR_67",  "KEEP_LAST"},
        {"XML_PROFILES_ENV_VAR_68",  "50"},
        {"XML_PROFILES_ENV_VAR_69",  "432"},
        {"XML_PROFILES_ENV_VAR_70",  "1"},
        {"XML_PROFILES_ENV_VAR_71",  "100"},
        {"XML_PROFILES_ENV_VAR_72",  "123"},
        {"XML_PROFILES_ENV_VAR_73",  "TRANSIENT_LOCAL"},
        {"XML_PROFILES_ENV_VAR_74",  "MANUAL_BY_PARTICIPANT"},
        {"XML_PROFILES_ENV_VAR_75",  "1"},
        {"XML_PROFILES_ENV_VAR_76",  "2"},
        {"XML_PROFILES_ENV_VAR_77",  "DURATION_INFINITY"},
        {"XML_PROFILES_ENV_VAR_78",  "BEST_EFFORT"},
        {"XML_PROFILES_ENV_VAR_79",  "0"},
        {"XML_PROFILES_ENV_VAR_80",  "0"},
        {"XML_PROFILES_ENV_VAR_81",  "partition_name_a"},
        {"XML_PROFILES_ENV_VAR_82",  "partition_name_b"},
        {"XML_PROFILES_ENV_VAR_83",  "ASYNCHRONOUS"},
        {"XML_PROFILES_ENV_VAR_84",  "56.30.0.1"},
        {"XML_PROFILES_ENV_VAR_85",  "5.3.1.0"},
        {"XML_PROFILES_ENV_VAR_86",  "5.3.1.0.F1"},
        {"XML_PROFILES_ENV_VAR_87",  "0"},
        {"XML_PROFILES_ENV_VAR_88",  "0"},
        {"XML_PROFILES_ENV_VAR_89",  "11"},
        {"XML_PROFILES_ENV_VAR_90",  "32"},
        {"XML_PROFILES_ENV_VAR_91",  "0"},
        {"XML_PROFILES_ENV_VAR_92",  "0"},
        {"XML_PROFILES_ENV_VAR_93",  "121"},
        {"XML_PROFILES_ENV_VAR_94",  "332"},
        {"XML_PROFILES_ENV_VAR_95",  "192.168.1.3"},
        {"XML_PROFILES_ENV_VAR_96",  "197"},
        {"XML_PROFILES_ENV_VAR_97",  "192.168.1.9"},
        {"XML_PROFILES_ENV_VAR_98",  "219"},
        {"XML_PROFILES_ENV_VAR_99",  "239.255.0.1"},
        {"XML_PROFILES_ENV_VAR_100", "2020"},
        {"XML_PROFILES_ENV_VAR_101", ""},
        {"XML_PROFILES_ENV_VAR_102", "1989"},
        {"XML_PROFILES_ENV_VAR_103", "true"},
        {"XML_PROFILES_ENV_VAR_104", "10.10.10.10"},
        {"XML_PROFILES_ENV_VAR_105", "2001"},
        {"XML_PROFILES_ENV_VAR_106", "DYNAMIC"},
        {"XML_PROFILES_ENV_VAR_107", "67"},
        {"XML_PROFILES_ENV_VAR_108", "87"},
        {"XML_PROFILES_ENV_VAR_109", "10"},
        {"XML_PROFILES_ENV_VAR_110", "10"},
        {"XML_PROFILES_ENV_VAR_111", "0"},
        {"XML_PROFILES_ENV_VAR_112", "KEEP_ALL"},
        {"XML_PROFILES_ENV_VAR_113", "1001"},
        {"XML_PROFILES_ENV_VAR_114", "52"},
        {"XML_PROFILES_ENV_VAR_115", "25"},
        {"XML_PROFILES_ENV_VAR_116", "32"},
        {"XML_PROFILES_ENV_VAR_117", "37"},
        {"XML_PROFILES_ENV_VAR_118", "PERSISTENT"},
        {"XML_PROFILES_ENV_VAR_119", "MANUAL_BY_TOPIC"},
        {"XML_PROFILES_ENV_VAR_120", "11"},
        {"XML_PROFILES_ENV_VAR_121", "22"},
        {"XML_PROFILES_ENV_VAR_122", "0"},
        {"XML_PROFILES_ENV_VAR_123", "0"},
        {"XML_PROFILES_ENV_VAR_124", "RELIABLE"},
        {"XML_PROFILES_ENV_VAR_125", "DURATION_INFINITY"},
        {"XML_PROFILES_ENV_VAR_126", "partition_name_c"},
        {"XML_PROFILES_ENV_VAR_127", "partition_name_d"},
        {"XML_PROFILES_ENV_VAR_128", "partition_name_e"},
        {"XML_PROFILES_ENV_VAR_129", "partition_name_f"},
        {"XML_PROFILES_ENV_VAR_130", "56.30.0.1"},
        {"XML_PROFILES_ENV_VAR_131", "5.3.1.0"},
        {"XML_PROFILES_ENV_VAR_132", "5.3.1.0.F1"},
        {"XML_PROFILES_ENV_VAR_133", "0"},
        {"XML_PROFILES_ENV_VAR_134", "0"},
        {"XML_PROFILES_ENV_VAR_135", "18"},
        {"XML_PROFILES_ENV_VAR_136", "81"},
        {"XML_PROFILES_ENV_VAR_137", "192.168.1.10"},
        {"XML_PROFILES_ENV_VAR_138", "196"},
        {"XML_PROFILES_ENV_VAR_139", "212"},
        {"XML_PROFILES_ENV_VAR_140", "239.255.0.10"},
        {"XML_PROFILES_ENV_VAR_141", "220"},
        {"XML_PROFILES_ENV_VAR_142", "239.255.0.11"},
        {"XML_PROFILES_ENV_VAR_143", "9891"},
        {"XML_PROFILES_ENV_VAR_144", "true"},
        {"XML_PROFILES_ENV_VAR_145", "10.10.10.10"},
        {"XML_PROFILES_ENV_VAR_146", "2001"},
        {"XML_PROFILES_ENV_VAR_147", "true"},
        {"XML_PROFILES_ENV_VAR_148", "PREALLOCATED_WITH_REALLOC"},
        {"XML_PROFILES_ENV_VAR_149", "13"},
        {"XML_PROFILES_ENV_VAR_150", "31"},
        {"XML_PROFILES_ENV_VAR_151", "10"},
        {"XML_PROFILES_ENV_VAR_152", "10"},
        {"XML_PROFILES_ENV_VAR_153", "0"},
        {"XML_PROFILES_ENV_VAR_154", "KEEP_ALL"},
        {"XML_PROFILES_ENV_VAR_155", "1001"},
        {"XML_PROFILES_ENV_VAR_156", "FULL"},
        {"XML_PROFILES_ENV_VAR_157", "true"},
        {"XML_PROFILES_ENV_VAR_158", "-1"},
        {"XML_PROFILES_ENV_VAR_159", "0"},
        {"XML_PROFILES_ENV_VAR_160", "0"},
        {"XML_PROFILES_ENV_VAR_161", "-1"},
        {"XML_PROFILES_ENV_VAR_162", "ON"},
        {"XML_PROFILES_ENV_VAR_163", "test_flow_controller"},
        {"XML_PROFILES_ENV_VAR_164", "HIGH_PRIORITY"},
        {"XML_PROFILES_ENV_VAR_165", "2048"},
        {"XML_PROFILES_ENV_VAR_166",  "45"},
        {"XML_PROFILES_ENV_VAR_167",  "test_flow_controller"},
        {"XML_PROFILES_ENV_VAR_168",  "251"},
        {"XML_PROFILES_ENV_VAR_169",  "127.0.0.1"}
    };

};

static void check_external_locator(
        const eprosima::fastdds::rtps::ExternalLocators& external_locators,
        uint8_t externality,
        uint8_t cost,
        uint8_t mask,
        const char* address,
        uint32_t port)
{
    auto ext_it = external_locators.find(externality);
    ASSERT_NE(ext_it, external_locators.end());
    auto cost_it = ext_it->second.find(cost);
    ASSERT_NE(cost_it, ext_it->second.end());
    for (const eprosima::fastdds::rtps::LocatorWithMask& loc : cost_it->second)
    {
        if (IPLocator::ip_to_string(loc).compare(address) == 0)
        {
            EXPECT_EQ(mask, loc.mask());
            EXPECT_EQ(port, loc.port);
            return;
        }
    }

    EXPECT_FALSE(true);
}

TEST_F(XMLProfileParserBasicTests, XMLParserRootLibrarySettings)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_root_library_settings_profile.xml"));

    const eprosima::fastdds::LibrarySettings& library_settings = xmlparser::XMLProfileManager::library_settings();
    EXPECT_EQ(library_settings.intraprocess_delivery,
            eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY);
}

TEST_F(XMLProfileParserBasicTests, XMLoadProfiles)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_profile.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_security_profile.xml"));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
            xmlparser::XMLProfileManager::loadXMLFile("missing_file.xml"));

    xmlparser::ParticipantAttributes participant_atts;
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillParticipantAttributes("test_participant_profile", participant_atts));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
            xmlparser::XMLProfileManager::fillParticipantAttributes("bad_name", participant_atts));
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_ERROR,
            xmlparser::XMLProfileManager::fillParticipantAttributes("test_publisher_profile", participant_atts));
}

/*
 * This test checks the loadXMLProfiles function```
 * 1. Check correct parsing of an XMLElement
 * 2. that an error is returned when trying to parse an XMLElement with an unknown tag.
 * 3. Check return on parsing of an XMLElement with an empty profiles tag
 * 4. Check return on parsing of an XMLElement with a root element different from <profiles>
 */
TEST_F(XMLProfileParserBasicTests, loadXMLProfiles)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // minimal profile xml
        std::string xml_profile =
                "\
                <profiles>\
                    <publisher profile_name=\"test_publisher_profile\"\
                    is_default_profile=\"true\">\
                        <qos>\
                            <durability>\
                                <kind>TRANSIENT_LOCAL</kind>\
                            </durability>\
                        </qos>\
                    </publisher>\
                </profiles>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_profile.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLProfiles(*titleElement));
        xmlparser::XMLProfileManager::DeleteInstance();

    }

    {
        std::vector<std::string> errors =
        {
            "<profiles><bad_tag></bad_tag></profiles>", // wrong profile xml
            "<profiles></profiles>", // empty profile xml
            "<dds></dds>" // not a profile tag as root
        };

        for (std::string& xml_profile : errors)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_profile.c_str()));
            titleElement = xml_doc.RootElement();
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLProfiles(*titleElement));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }
}

/*
 * This test checks the return of the loadXMLDynamicTypes method
 * 1. Check correct return
 * 2. Check error return
 */
TEST_F(XMLProfileParserBasicTests, loadXMLDynamicTypes)
{
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLElement* titleElement;

    {
        // minimal types xml
        std::string xml =
                "\
                <types>\
                    <type>\
                        <enum name=\"MyAloneEnumType\">\
                            <enumerator name=\"A\" value=\"0\"/>\
                            <enumerator name=\"B\" value=\"1\"/>\
                        </enum>\
                    </type>\
                </types>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLDynamicTypes(*titleElement));
    }

    {
        // wrong xml
        std::string xml =
                "\
                <types>\
                    <type>\
                        <bad_tag></bad_tag>\
                    </type>\
                </types>\
                ";

        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        titleElement = xml_doc.RootElement();
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLDynamicTypes(*titleElement));
    }
}

TEST_P(XMLProfileParserTests, XMLParserLibrarySettings)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));

    const eprosima::fastdds::LibrarySettings& library_settings = xmlparser::XMLProfileManager::library_settings();
    EXPECT_EQ(library_settings.intraprocess_delivery, eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL);
}

/*
 * Checks the XML validated participant parsing
 */
TEST_P(XMLProfileParserTests, XMLParserParticipant)
{
    std::string participant_profile = std::string("test_participant_profile");
    xmlparser::ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    EXPECT_EQ(participant_atts.domainId, 123u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    EXPECT_EQ(rtps_atts.allocation.locators.max_unicast_locators, 4u);
    EXPECT_EQ(rtps_atts.allocation.locators.max_multicast_locators, 1u);
    EXPECT_EQ(rtps_atts.allocation.participants.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.participants.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.participants.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.readers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.readers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.readers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.writers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.writers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.writers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.preallocated_number, 127u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.dynamic, true);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.network_buffers_config.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.network_buffers_config.maximum, 127u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.network_buffers_config.increment, 10u);

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    check_external_locator(rtps_atts.default_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    EXPECT_TRUE(rtps_atts.ignore_non_matching_locators);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS |
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    check_external_locator(rtps_atts.builtin.metatraffic_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2002);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(rtps_atts.netmaskFilter, eprosima::fastdds::rtps::NetmaskFilterKind::ON);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

/*
 * Checks the participant parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserParticipantDeprecated)
{
    std::string participant_profile = std::string("test_participant_profile");
    xmlparser::ParticipantAttributes participant_atts;

    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    EXPECT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    EXPECT_EQ(rtps_atts.allocation.locators.max_unicast_locators, 4u);
    EXPECT_EQ(rtps_atts.allocation.locators.max_multicast_locators, 1u);
    EXPECT_EQ(rtps_atts.allocation.participants.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.participants.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.participants.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.readers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.readers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.readers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.writers.initial, 10u);
    EXPECT_EQ(rtps_atts.allocation.writers.maximum, 20u);
    EXPECT_EQ(rtps_atts.allocation.writers.increment, 2u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.preallocated_number, 127u);
    EXPECT_EQ(rtps_atts.allocation.send_buffers.dynamic, true);

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    check_external_locator(rtps_atts.default_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    EXPECT_TRUE(rtps_atts.ignore_non_matching_locators);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS |
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    check_external_locator(rtps_atts.builtin.metatraffic_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2002);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

/*
 * Checks the XML validated participant default profile parsing
 */
TEST_P(XMLProfileParserTests, XMLParserDefaultParticipantProfile)
{
    xmlparser::ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts);

    EXPECT_EQ(participant_atts.domainId, 123u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    check_external_locator(rtps_atts.default_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    EXPECT_TRUE(rtps_atts.ignore_non_matching_locators);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS |
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    check_external_locator(rtps_atts.builtin.metatraffic_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2002);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

/*
 * Checks the participant default profile parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserDefaultParticipantProfileDeprecated)
{
    xmlparser::ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts);

    EXPECT_EQ(participant_atts.domainId, 2019102u);
    RTPSParticipantAttributes& rtps_atts = participant_atts.rtps;
    BuiltinAttributes& builtin = rtps_atts.builtin;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    PortParameters& port = rtps_atts.port;

    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2019;
    EXPECT_EQ(*rtps_atts.defaultUnicastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2021;
    EXPECT_EQ(*rtps_atts.defaultMulticastLocatorList.begin(), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 1);
    locator.port = 1979;
    check_external_locator(rtps_atts.default_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    EXPECT_TRUE(rtps_atts.ignore_non_matching_locators);
    EXPECT_EQ(rtps_atts.sendSocketBufferSize, 32u);
    EXPECT_EQ(rtps_atts.listenSocketBufferSize, 1000u);
    EXPECT_EQ(builtin.discovery_config.discoveryProtocol, eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE);
    EXPECT_EQ(builtin.discovery_config.ignoreParticipantFlags,
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS |
            eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST);
    EXPECT_EQ(builtin.use_WriterLivelinessProtocol, false);
    EXPECT_EQ(builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol, true);
    EXPECT_EQ(builtin.discovery_config.use_STATIC_EndpointDiscoveryProtocol, false);
    EXPECT_EQ(builtin.discovery_config.leaseDuration, dds::c_TimeInfinite);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.seconds, 10);
    EXPECT_EQ(builtin.discovery_config.leaseDuration_announcementperiod.nanosec, 333u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.count, 2u);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.seconds, 1);
    EXPECT_EQ(builtin.discovery_config.initial_announcements.period.nanosec, 827u);
    EXPECT_FALSE(builtin.avoid_builtin_multicast);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, false);
    EXPECT_EQ(builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, true);
    check_external_locator(rtps_atts.builtin.metatraffic_external_unicast_locators, 100, 200, 10, "10.10.10.10", 2002);
    IPLocator::setIPv4(locator, 192, 168, 1, 5);
    locator.port = 9999;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficUnicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 6);
    locator.port = 6666;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 2);
    locator.port = 32;
    EXPECT_EQ(*(loc_list_it = builtin.metatrafficMulticastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 3);
    locator.port = 2112;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 21120;
    EXPECT_EQ(*(loc_list_it = builtin.initialPeersList.begin()), locator);
    EXPECT_EQ(builtin.readerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.writerHistoryMemoryPolicy, PREALLOCATED_MEMORY_MODE);
    EXPECT_EQ(builtin.readerPayloadSize, 1000u);
    EXPECT_EQ(builtin.writerPayloadSize, 2000u);
    EXPECT_EQ(builtin.mutation_tries, 55u);
    EXPECT_EQ(port.portBase, 12);
    EXPECT_EQ(port.domainIDGain, 34);
    EXPECT_EQ(port.participantIDGain, 56);
    EXPECT_EQ(port.offsetd0, 78);
    EXPECT_EQ(port.offsetd1, 90);
    EXPECT_EQ(port.offsetd2, 123);
    EXPECT_EQ(port.offsetd3, 456);
    EXPECT_EQ(port.offsetd4, 251);
    EXPECT_EQ(rtps_atts.participantID, 9898);
    EXPECT_EQ(rtps_atts.easy_mode_ip, "127.0.0.1");
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->max_bytes_per_period, 2048);
    EXPECT_EQ(rtps_atts.flow_controllers.at(0)->period_ms, 45u);
    EXPECT_EQ(rtps_atts.useBuiltinTransports, true);
    EXPECT_EQ(std::string(rtps_atts.getName()), "test_name");
}

/*
 * Checks the XML validated data writer parsing
 */
TEST_P(XMLProfileParserTests, XMLParserPublisher)
{
    std::string publisher_profile = std::string("test_publisher_profile");
    xmlparser::PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_atts));

    xmlparser::TopicAttributes& pub_topic = publisher_atts.topic;
    dds::WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

    //EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    //EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    //EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, dds::c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, dds::BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, dds::c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_qos.m_publishMode.flow_controller_name, "test_flow_controller");
    EXPECT_EQ(pub_times.initial_heartbeat_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.heartbeat_period.seconds, 11);
    EXPECT_EQ(pub_times.heartbeat_period.nanosec, 32u);
    EXPECT_EQ(pub_times.nack_response_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.nack_supression_duration.seconds, 121);
    EXPECT_EQ(pub_times.nack_supression_duration.nanosec, 332u);
    EXPECT_TRUE(publisher_atts.ignore_non_matching_locators);
    check_external_locator(publisher_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the data writer parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserPublisherDeprecated)
{
    std::string publisher_profile = std::string("test_publisher_profile");
    xmlparser::PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_atts));

    xmlparser::TopicAttributes& pub_topic = publisher_atts.topic;
    dds::WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

    EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, dds::c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, dds::BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, dds::c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_qos.m_publishMode.flow_controller_name, "test_flow_controller");
    EXPECT_EQ(pub_times.initial_heartbeat_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.heartbeat_period.seconds, 11);
    EXPECT_EQ(pub_times.heartbeat_period.nanosec, 32u);
    EXPECT_EQ(pub_times.nack_response_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.nack_supression_duration.seconds, 121);
    EXPECT_EQ(pub_times.nack_supression_duration.nanosec, 332u);
    EXPECT_TRUE(publisher_atts.ignore_non_matching_locators);
    check_external_locator(publisher_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the XML validated data writer default profile parsing
 */
TEST_P(XMLProfileParserTests, XMLParserDefaultPublisherProfile)
{
    xmlparser::PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    xmlparser::XMLProfileManager::getDefaultPublisherAttributes(publisher_atts);

    xmlparser::TopicAttributes& pub_topic = publisher_atts.topic;
    dds::WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

    //EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    //EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    //EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, dds::c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, dds::BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, dds::c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_qos.m_publishMode.flow_controller_name, "test_flow_controller");
    EXPECT_EQ(pub_times.initial_heartbeat_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.heartbeat_period.seconds, 11);
    EXPECT_EQ(pub_times.heartbeat_period.nanosec, 32u);
    EXPECT_EQ(pub_times.nack_response_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.nack_supression_duration.seconds, 121);
    EXPECT_EQ(pub_times.nack_supression_duration.nanosec, 332u);
    EXPECT_TRUE(publisher_atts.ignore_non_matching_locators);
    check_external_locator(publisher_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the data writer default profile parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserDefaultPublisherProfileDeprecated)
{
    xmlparser::PublisherAttributes publisher_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    xmlparser::XMLProfileManager::getDefaultPublisherAttributes(publisher_atts);

    xmlparser::TopicAttributes& pub_topic = publisher_atts.topic;
    dds::WriterQos& pub_qos = publisher_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    WriterTimes& pub_times = publisher_atts.times;

    EXPECT_EQ(pub_topic.topicKind, NO_KEY);
    EXPECT_EQ(pub_topic.topicName, "samplePubSubTopic");
    EXPECT_EQ(pub_topic.topicDataType, "samplePubSubTopicType");
    EXPECT_EQ(pub_topic.historyQos.kind, dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_EQ(pub_topic.historyQos.depth, 50);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples, 432);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_instances, 1);
    EXPECT_EQ(pub_topic.resourceLimitsQos.max_samples_per_instance, 100);
    EXPECT_EQ(pub_topic.resourceLimitsQos.allocated_samples, 123);
    EXPECT_EQ(pub_qos.m_durability.kind, dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.kind, dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.seconds, 1);
    EXPECT_EQ(pub_qos.m_liveliness.lease_duration.nanosec, 2u);
    EXPECT_EQ(pub_qos.m_liveliness.announcement_period, dds::c_TimeInfinite);
    EXPECT_EQ(pub_qos.m_reliability.kind, dds::BEST_EFFORT_RELIABILITY_QOS);
    EXPECT_EQ(pub_qos.m_reliability.max_blocking_time, dds::c_TimeZero);
    EXPECT_EQ(pub_qos.m_partition.names()[0], "partition_name_a");
    EXPECT_EQ(pub_qos.m_partition.names()[1], "partition_name_b");
    EXPECT_EQ(pub_qos.m_publishMode.kind, dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_EQ(pub_qos.m_publishMode.flow_controller_name, "test_flow_controller");
    EXPECT_EQ(pub_times.initial_heartbeat_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.heartbeat_period.seconds, 11);
    EXPECT_EQ(pub_times.heartbeat_period.nanosec, 32u);
    EXPECT_EQ(pub_times.nack_response_delay, dds::c_TimeZero);
    EXPECT_EQ(pub_times.nack_supression_duration.seconds, 121);
    EXPECT_EQ(pub_times.nack_supression_duration.nanosec, 332u);
    EXPECT_TRUE(publisher_atts.ignore_non_matching_locators);
    check_external_locator(publisher_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 3);
    locator.port = 197;
    EXPECT_EQ(*(loc_list_it = publisher_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 9);
    locator.port = 219;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = 2020;
    EXPECT_EQ(*(loc_list_it = publisher_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    locator.port = 1989;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    //locator.port = 2021;
    //EXPECT_EQ(*(loc_list_it = publisher_atts.outLocatorList.begin()), locator);
    //EXPECT_EQ(loc_list_it->get_port(), 2021);
    EXPECT_EQ(publisher_atts.historyMemoryPolicy, DYNAMIC_RESERVE_MEMORY_MODE);
    EXPECT_EQ(publisher_atts.getUserDefinedID(), 67);
    EXPECT_EQ(publisher_atts.getEntityID(), 87);
    EXPECT_EQ(publisher_atts.matched_subscriber_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the XML validated data reader parsing
 */
TEST_P(XMLProfileParserTests, XMLParserSubscriber)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    xmlparser::SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    xmlparser::TopicAttributes& sub_topic = subscriber_atts.topic;
    dds::ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

    //EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    //EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    //EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, dds::KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, dds::c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, dds::c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initial_acknack_delay, dds::c_TimeZero);
    EXPECT_EQ(sub_times.heartbeat_response_delay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeat_response_delay.nanosec, 81u);
    EXPECT_TRUE(subscriber_atts.ignore_non_matching_locators);
    check_external_locator(subscriber_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the data reader parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserSubscriberDeprecated)
{
    std::string subscriber_profile = std::string("test_subscriber_profile");
    xmlparser::SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    xmlparser::TopicAttributes& sub_topic = subscriber_atts.topic;
    dds::ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

    EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, dds::KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, dds::c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, dds::c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initial_acknack_delay, dds::c_TimeZero);
    EXPECT_EQ(sub_times.heartbeat_response_delay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeat_response_delay.nanosec, 81u);
    EXPECT_TRUE(subscriber_atts.ignore_non_matching_locators);
    check_external_locator(subscriber_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the XML validated data reader default profile parsing
 */
TEST_P(XMLProfileParserTests, XMLParserDefaultSubscriberProfile)
{
    xmlparser::SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile(xml_filename_));
    xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(subscriber_atts);

    xmlparser::TopicAttributes& sub_topic = subscriber_atts.topic;
    dds::ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

    //EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    //EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    //EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, dds::KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, dds::c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, dds::c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initial_acknack_delay, dds::c_TimeZero);
    EXPECT_EQ(sub_times.heartbeat_response_delay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeat_response_delay.nanosec, 81u);
    EXPECT_TRUE(subscriber_atts.ignore_non_matching_locators);
    check_external_locator(subscriber_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Checks the data reader default profile parsing (with deprecated but supported elements)
 */
TEST_F(XMLProfileParserBasicTests, XMLParserDefaultSubscriberProfileDeprecated)
{
    xmlparser::SubscriberAttributes subscriber_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));
    xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(subscriber_atts);

    xmlparser::TopicAttributes& sub_topic = subscriber_atts.topic;
    dds::ReaderQos& sub_qos = subscriber_atts.qos;
    Locator_t locator;
    LocatorListIterator loc_list_it;
    ReaderTimes& sub_times = subscriber_atts.times;

    EXPECT_EQ(sub_topic.topicKind, WITH_KEY);
    EXPECT_EQ(sub_topic.topicName, "otherSamplePubSubTopic");
    EXPECT_EQ(sub_topic.topicDataType, "otherSamplePubSubTopicType");
    EXPECT_EQ(sub_topic.historyQos.kind, dds::KEEP_ALL_HISTORY_QOS);
    EXPECT_EQ(sub_topic.historyQos.depth, 1001);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples, 52);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_instances, 25);
    EXPECT_EQ(sub_topic.resourceLimitsQos.max_samples_per_instance, 32);
    EXPECT_EQ(sub_topic.resourceLimitsQos.allocated_samples, 37);
    EXPECT_EQ(sub_qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.seconds, 11);
    EXPECT_EQ(sub_qos.m_liveliness.lease_duration.nanosec, 22u);
    EXPECT_EQ(sub_qos.m_liveliness.announcement_period, dds::c_TimeZero);
    EXPECT_EQ(sub_qos.m_reliability.kind, dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_EQ(sub_qos.m_reliability.max_blocking_time, dds::c_TimeInfinite);
    EXPECT_EQ(sub_qos.m_partition.names()[0], "partition_name_c");
    EXPECT_EQ(sub_qos.m_partition.names()[1], "partition_name_d");
    EXPECT_EQ(sub_qos.m_partition.names()[2], "partition_name_e");
    EXPECT_EQ(sub_qos.m_partition.names()[3], "partition_name_f");
    EXPECT_EQ(sub_times.initial_acknack_delay, dds::c_TimeZero);
    EXPECT_EQ(sub_times.heartbeat_response_delay.seconds, 18);
    EXPECT_EQ(sub_times.heartbeat_response_delay.nanosec, 81u);
    EXPECT_TRUE(subscriber_atts.ignore_non_matching_locators);
    check_external_locator(subscriber_atts.external_unicast_locators, 100, 200, 10, "10.10.10.10", 2001);
    IPLocator::setIPv4(locator, 192, 168, 1, 10);
    locator.port = 196;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.unicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 212;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 10);
    locator.port = 220;
    EXPECT_EQ(*(loc_list_it = subscriber_atts.multicastLocatorList.begin()), locator);
    IPLocator::setIPv4(locator, 0, 0, 0, 0);
    locator.port = 0;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 239, 255, 0, 11);
    locator.port = 9891;
    ++loc_list_it;
    EXPECT_EQ(*loc_list_it, locator);
    IPLocator::setIPv4(locator, 192, 168, 1, 2);
    locator.port = 2079;
    //EXPECT_EQ(*(loc_list_it = subscriber_atts.outLocatorList.begin()), locator);
    EXPECT_EQ(subscriber_atts.historyMemoryPolicy, PREALLOCATED_WITH_REALLOC_MEMORY_MODE);
    EXPECT_EQ(subscriber_atts.getUserDefinedID(), 13);
    EXPECT_EQ(subscriber_atts.getEntityID(), 31);
    EXPECT_EQ(subscriber_atts.matched_publisher_allocation, ResourceLimitedContainerConfig::fixed_size_configuration(
                10u));
}

/*
 * Tests the correct parsing of a requester XML profile
 * 1. Check if attributes were filled correctly
 * 2. Check error return on a missing requester profile name
 */
TEST_F(XMLProfileParserBasicTests, XMLParserRequesterProfile)
{
    std::string requester_profile = std::string("test_requester_profile");
    xmlparser::RequesterAttributes requester_atts;

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::fillRequesterAttributes(requester_profile, requester_atts));

    xmlparser::PublisherAttributes& publisher_atts = requester_atts.publisher;
    xmlparser::SubscriberAttributes& subscriber_atts = requester_atts.subscriber;

    EXPECT_EQ(publisher_atts.topic.topicDataType, "request_type");
    EXPECT_EQ(publisher_atts.topic.topicName, "service_name_Request");
    EXPECT_EQ(publisher_atts.qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);

    EXPECT_EQ(subscriber_atts.topic.topicDataType, "reply_type");
    EXPECT_EQ(subscriber_atts.topic.topicName, "service_name_Reply");
    EXPECT_EQ(subscriber_atts.qos.m_durability.kind, dds::PERSISTENT_DURABILITY_QOS);

    // Wrong profile name
    std::string missing_profile = std::string("missing_profile");
    EXPECT_EQ(
        xmlparser::XMLP_ret::XML_ERROR,
        xmlparser::XMLProfileManager::fillRequesterAttributes(missing_profile, requester_atts)
        );
}

/*
 * Tests the correct parsing of a replier XML profile
 * 1. Check if attributes were filled correctly
 * 2. Check error return on a missing replier profile name
 */
TEST_F(XMLProfileParserBasicTests, XMLParserReplierProfile)
{
    std::string replier_profile = std::string("test_replier_profile");
    xmlparser::ReplierAttributes replier_atts;

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::loadXMLFile("test_xml_deprecated.xml"));

    ASSERT_EQ(
        xmlparser::XMLP_ret::XML_OK,
        xmlparser::XMLProfileManager::fillReplierAttributes(replier_profile, replier_atts));

    xmlparser::PublisherAttributes& publisher_atts = replier_atts.publisher;
    xmlparser::SubscriberAttributes& subscriber_atts = replier_atts.subscriber;

    EXPECT_EQ(publisher_atts.topic.topicDataType, "reply_type");
    EXPECT_EQ(publisher_atts.topic.topicName, "reply_topic_name");
    EXPECT_EQ(publisher_atts.qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);

    EXPECT_EQ(subscriber_atts.topic.topicDataType, "request_type");
    EXPECT_EQ(subscriber_atts.topic.topicName, "request_topic_name");
    EXPECT_EQ(subscriber_atts.qos.m_liveliness.kind, dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);

    // Wrong profile name
    std::string missing_profile = std::string("missing_profile");
    EXPECT_EQ(
        xmlparser::XMLP_ret::XML_ERROR,
        xmlparser::XMLProfileManager::fillReplierAttributes(missing_profile, replier_atts)
        );
}

#if HAVE_SECURITY

TEST_F(XMLProfileParserBasicTests, XMLParserSecurity)
{
    std::string participant_profile = std::string("test_participant_security_profile");
    xmlparser::ParticipantAttributes participant_atts;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("test_xml_security_profile.xml"));
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillParticipantAttributes(participant_profile, participant_atts));

    PropertySeq& part_props = participant_atts.rtps.properties.properties();
    BinaryPropertySeq& part_bin_props = participant_atts.rtps.properties.binary_properties();

    EXPECT_EQ(part_props[0].name(), "dds.sec.auth.builtin.PKI-DH.identity_ca");
    EXPECT_EQ(part_props[0].value(), "maincacert.pem");
    EXPECT_EQ(part_props[0].propagate(), false);
    EXPECT_EQ(part_props[1].name(), "dds.sec.auth.builtin.PKI-DH.identity_certificate");
    EXPECT_EQ(part_props[1].value(), "appcert.pem");
    EXPECT_EQ(part_props[1].propagate(), true);
    EXPECT_EQ(part_bin_props[0].name(), "binary_prop_a");
    EXPECT_EQ(part_bin_props[0].propagate(), false);
    EXPECT_EQ(part_bin_props[1].name(), "binary_prop_b");
    EXPECT_EQ(part_bin_props[1].propagate(), true);


    std::string publisher_profile = std::string("test_publisher_security_profile");
    xmlparser::PublisherAttributes publisher_atts;
    EXPECT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillPublisherAttributes(publisher_profile, publisher_atts));

    PropertySeq& pub_props = publisher_atts.properties.properties();
    BinaryPropertySeq& pub_bin_props = publisher_atts.properties.binary_properties();

    EXPECT_EQ(pub_props[0].name(), "rtps.endpoint.submessage_protection_kind");
    EXPECT_EQ(pub_props[0].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[0].propagate(), true);
    EXPECT_EQ(pub_props[1].name(), "rtps.endpoint.payload_protection_kind");
    EXPECT_EQ(pub_props[1].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[1].propagate(), true);
    EXPECT_EQ(pub_bin_props[0].name(), "binary_prop_c");
    EXPECT_EQ(pub_bin_props[0].propagate(), true);
    EXPECT_EQ(pub_bin_props[1].name(), "binary_prop_d");
    EXPECT_EQ(pub_bin_props[1].propagate(), false);


    std::string subscriber_profile = std::string("test_subscriber_security_profile");
    xmlparser::SubscriberAttributes subscriber_atts;

    EXPECT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::fillSubscriberAttributes(subscriber_profile, subscriber_atts));

    PropertySeq& sub_props = subscriber_atts.properties.properties();
    BinaryPropertySeq& sub_bin_props = subscriber_atts.properties.binary_properties();

    EXPECT_EQ(sub_props[0].name(), "rtps.endpoint.submessage_protection_kind");
    EXPECT_EQ(pub_props[0].value(), "ENCRYPT");
    EXPECT_EQ(pub_props[0].propagate(), true);
    EXPECT_EQ(sub_props[1].name(), "rtps.endpoint.payload_protection_kind");
    EXPECT_EQ(sub_props[1].value(), "ENCRYPT");
    EXPECT_EQ(sub_props[1].propagate(), true);
    EXPECT_EQ(sub_bin_props[0].name(), "binary_prop_e");
    EXPECT_EQ(sub_bin_props[0].propagate(), true);
    EXPECT_EQ(sub_bin_props[1].name(), "binary_prop_f");
    EXPECT_EQ(sub_bin_props[1].propagate(), false);
}

#endif // if HAVE_SECURITY

TEST_F(XMLProfileParserBasicTests, file_xml_consumer_append)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsFileConsumer())).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_node_file_append_profile.xml");
}

TEST_F(XMLProfileParserBasicTests, log_inactive)
{
    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_inactive_profile.xml");
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it sets
 * a `stderr_threshold` to`Log::Kind::Error` using a property `stderr_threshold`. The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_OK`, meaning that the property was correctly set.
 */
TEST_F(XMLProfileParserBasicTests, log_register_stdouterr)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastdds::xmlparser::XMLP_ret ret =
            xmlparser::XMLProfileManager::loadXMLFile("log_stdouterr_profile.xml");
    ASSERT_EQ(eprosima::fastdds::xmlparser::XMLP_ret::XML_OK, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using a property `threshold` (which is not the correct
 * property name). The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set.
 */
TEST_F(XMLProfileParserBasicTests, log_register_stdouterr_wrong_property_name)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastdds::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_wrong_property_name_profile_invalid.xml");
    ASSERT_EQ(eprosima::fastdds::xmlparser::XMLP_ret::XML_ERROR, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using a property `stderr_threshold` with value `Error`
 * (which is not a correct property value). The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set.
 */
TEST_F(XMLProfileParserBasicTests, log_register_stdouterr_wrong_property_value)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastdds::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_wrong_property_value_profile_invalid.xml");
    ASSERT_EQ(eprosima::fastdds::xmlparser::XMLP_ret::XML_ERROR, ret);
}

/*
 * This test registers a StdoutErrConsumer using XML and setting the `use_default` flag to FALSE. Furthermore, it
 * attempts to set a `stderr_threshold` to`Log::Kind::Error` using two properties `stderr_threshold` with different
 * values. However, this operation is not permited, since only one property with name `` can be present.
 * The test checks that:
 *    1. `ClearConsumers()` is called (because `use_default` is set to FALSE).
 *    2. `RegisterConsumer()` is called, meaning a `StdoutErrConsumer` was registered.
 *    3. The return code of `loadXMLFile` is `XMLP_ret::XML_ERROR`, meaning that the property was NOT correctly set. In
 *       this case, the first `stderr_threshold` property will be used, but the function will warn of an incorrect XML
 *       configuration.
 */
TEST_F(XMLProfileParserBasicTests, log_register_stdouterr_two_thresholds)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(1);
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(1);
    eprosima::fastdds::xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLFile(
        "log_stdouterr_two_thresholds_profile.xml");
    ASSERT_EQ(eprosima::fastdds::xmlparser::XMLP_ret::XML_ERROR, ret);
}

TEST_F(XMLProfileParserBasicTests, file_and_default)
{
    using namespace eprosima::fastdds::dds;

    EXPECT_CALL(*log_mock, RegisterConsumer(IsFileConsumer())).Times(1);
    EXPECT_CALL(*log_mock, SetThreadConfig()).Times(1);
    xmlparser::XMLProfileManager::loadXMLFile("log_def_file_profile.xml");
}

TEST_F(XMLProfileParserBasicTests, tls_config)
{
    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("tls_config_profile.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using TCPDescriptor = std::shared_ptr<eprosima::fastdds::rtps::TCPTransportDescriptor>;
    TCPDescriptor descriptor = std::dynamic_pointer_cast<eprosima::fastdds::rtps::TCPTransportDescriptor>(transport);

    /*
       <tls>
        <password>Password</password>
        <private_key_file>Key_file.pem</private_key_file>
        <cert_chain_file>Chain.pem</cert_chain_file>
        <tmp_dh_file>DH.pem</tmp_dh_file>
        <verify_file>verify.pem</verify_file>
        <verify_mode>VERIFY_PEER</verify_mode>
        <options>
            <option>NO_TLSV1</option>
            <option>NO_TLSV1_1</option>
        </options>
       </tls>
     */

    EXPECT_EQ("Password", descriptor->tls_config.password);
    EXPECT_EQ("Key_file.pem", descriptor->tls_config.private_key_file);
    EXPECT_EQ("RSA_file.pem", descriptor->tls_config.rsa_private_key_file);
    EXPECT_EQ("Chain.pem", descriptor->tls_config.cert_chain_file);
    EXPECT_EQ("DH.pem", descriptor->tls_config.tmp_dh_file);
    EXPECT_EQ("verify.pem", descriptor->tls_config.verify_file);
    EXPECT_EQ("my_server.com", descriptor->tls_config.server_name);
    EXPECT_EQ(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode::VERIFY_PEER,
            descriptor->tls_config.verify_mode);
    EXPECT_TRUE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions
                    ::NO_TLSV1));
    EXPECT_TRUE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions
                    ::NO_TLSV1_1));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::NO_SSLV2));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::NO_SSLV3));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::NO_TLSV1_2));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::NO_TLSV1_3));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::DEFAULT_WORKAROUNDS));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::NO_COMPRESSION));
    EXPECT_FALSE(descriptor->tls_config.get_option(eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::
                    TLSOptions::SINGLE_DH_USE));

    EXPECT_EQ(descriptor->tls_config.verify_paths.size(), static_cast<size_t>(3));
    EXPECT_EQ(descriptor->tls_config.verify_paths[0], "Path1");
    EXPECT_EQ(descriptor->tls_config.verify_paths[1], "Path2");
    EXPECT_EQ(descriptor->tls_config.verify_paths[2], "Path3");
    EXPECT_EQ(descriptor->tls_config.verify_depth, static_cast<int32_t>(55));
    EXPECT_TRUE(descriptor->tls_config.default_verify_path);

    EXPECT_EQ(descriptor->tls_config.handshake_role,
            eprosima::fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSHandShakeRole::SERVER);
}

TEST_F(XMLProfileParserBasicTests, UDP_transport_descriptors_config)
{
    using namespace eprosima::fastdds::rtps;

    ASSERT_EQ(  xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("UDP_transport_descriptors_config_profile.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using UDPDescriptor = std::shared_ptr<eprosima::fastdds::rtps::UDPTransportDescriptor>;
    UDPDescriptor descriptor = std::dynamic_pointer_cast<eprosima::fastdds::rtps::UDPTransportDescriptor>(transport);

    ASSERT_NE(descriptor, nullptr);
    EXPECT_EQ(descriptor->sendBufferSize, 8192u);
    EXPECT_EQ(descriptor->receiveBufferSize, 8192u);
    EXPECT_EQ(descriptor->TTL, 250u);
    EXPECT_EQ(descriptor->non_blocking_send, true);
    EXPECT_EQ(descriptor->maxMessageSize, 16384u);
    EXPECT_EQ(descriptor->maxInitialPeersRange, 100u);
    EXPECT_EQ(descriptor->interfaceWhiteList.size(), 2u);
    EXPECT_EQ(descriptor->interfaceWhiteList[0], "192.168.1.41");
    EXPECT_EQ(descriptor->interfaceWhiteList[1], "lo");
    EXPECT_EQ(descriptor->netmask_filter, NetmaskFilterKind::ON);
    EXPECT_EQ(descriptor->interface_allowlist[0], AllowedNetworkInterface("wlp59s0", NetmaskFilterKind::ON));
    EXPECT_EQ(descriptor->interface_allowlist[1], AllowedNetworkInterface("127.0.0.1", NetmaskFilterKind::AUTO));
    EXPECT_EQ(descriptor->interface_blocklist[0], BlockedNetworkInterface("docker0"));
    EXPECT_EQ(descriptor->m_output_udp_socket, 5101u);
}

TEST_F(XMLProfileParserBasicTests, SHM_transport_descriptors_config)
{
    ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
            xmlparser::XMLProfileManager::loadXMLFile("SHM_transport_descriptors_config_profile.xml"));

    xmlparser::sp_transport_t transport = xmlparser::XMLProfileManager::getTransportById("Test");

    using SHMDescriptor = std::shared_ptr<eprosima::fastdds::rtps::SharedMemTransportDescriptor>;
    SHMDescriptor descriptor = std::dynamic_pointer_cast<eprosima::fastdds::rtps::SharedMemTransportDescriptor>(
        transport);

    ASSERT_NE(descriptor, nullptr);
    ASSERT_EQ(descriptor->segment_size(), (std::numeric_limits<uint32_t>::max)());
    ASSERT_EQ(descriptor->port_queue_capacity(), (std::numeric_limits<uint32_t>::max)());
    ASSERT_EQ(descriptor->healthy_check_timeout_ms(), (std::numeric_limits<uint32_t>::max)());
    ASSERT_EQ(descriptor->rtps_dump_file(), "test_file.dump");
    ASSERT_EQ(descriptor->maxMessageSize, 128000u);
    ASSERT_EQ(descriptor->max_message_size(), 128000u);
}

/*
 * Test return code of the insertTransportById method when trying to insert two transports with the same id
 */
TEST_F(XMLProfileParserBasicTests, insertTransportByIdNegativeClauses)
{
    std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface> transport;
    EXPECT_EQ(true, xmlparser::XMLProfileManager::insertTransportById("duplicated_id", transport));
    EXPECT_EQ(false, xmlparser::XMLProfileManager::insertTransportById("duplicated_id", transport));
}

/*
 * Test return code of the getDynamicTypeBuilderByName method when trying to retrieve a type which has not been parsed
 */
TEST_F(XMLProfileParserBasicTests, getDynamicTypeBuilderByNameNegativeClausesNegativeClauses)
{
    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicTypeBuilder>::ref_type type_builder;
    EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR,
            xmlparser::XMLProfileManager::getDynamicTypeBuilderByName(type_builder, "wrong_type"));
    ASSERT_FALSE(type_builder);
}

/*
 * Test return code of the getTransportById method when trying to retrieve a transport descriptor which has not been
 * parsed
 */
TEST_F(XMLProfileParserBasicTests, getTransportByIdNegativeClauses)
{
    EXPECT_EQ(nullptr, xmlparser::XMLProfileManager::getTransportById("wrong_type"));
}

/*
 * Tests whether the extraction of XML profiles succeeds when all profiles are correct.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The following cases are tested:
 *  1. A corrrect standalone profile element is parsed, return value of XMLP_ret::XML_OK is expected
 *  2. A corrrect rooted profile element is parsed, return value of XMLP_ret::XML_OK is expected
 *  3. An incorrect profile element is parsed, return value of XMLP_ret::XML_ERROR is expected
 */
TEST_F(XMLProfileParserBasicTests, extract_profiles_ok)
{
    tinyxml2::XMLDocument xml_doc;
    {
        const char* xml =
                "                                                                                                            \n\
        <profiles>                                                                                                   \n\
            <participant profile_name=\"participant_prof\">                                                          \n\
                <rtps></rtps>                                                                                        \n\
            </participant>                                                                                           \n\
            <publisher profile_name=\"publisher_prof\"></publisher>                                                  \n\
            <subscriber profile_name=\"subscriber_prof\"></subscriber>                                               \n\
            <topic profile_name=\"topic_prof\"></topic>                                                              \n\
            <requester profile_name=\"req_prof\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">    \n\
            </requester>                                                                                             \n\
            <replier profile_name=\"replier_prof\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">  \n\
            </replier>                                                                                               \n\
        </profiles>                                                                                                  \n\
        ";

        // Standalone
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();

        // Rooted
        std::string xml_dds = "<dds>" + std::string(xml) + "</dds>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml_dds.c_str()));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }

    {
        // wrong xml
        std::string xml = "<bad_tag></bad_tag>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml.c_str()));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }


}

/*
 * Tests whether the extraction of XML profiles succeeds when some profiles are correct and some are not.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The expected return value is XMLP_ret::XML_NOK.
 * Checks a combination of a correct profile with an incorrect profile of each profile type
 */
TEST_F(XMLProfileParserBasicTests, extract_profiles_nok)
{
    tinyxml2::XMLDocument xml_doc;

    const char* xml_p =
            "\
                <profiles>\
                    <!-- OK PROFILE -->\
                    <participant profile_name=\"participant_prof\">\
                        <rtps></rtps>\
                    </participant>\
                    \
                    <!-- NOK PROFILE --> \
                    <%s></%s>\
                </profiles>\
            ";
    constexpr size_t xml_len {500};
    char xml[xml_len];

    std::vector<std::string> elements {
        "participant",
        "publisher",
        "subscriber",
        "topic",
        "requester",
        "replier",
    };

    for (const std::string& e : elements)
    {
        snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_NOK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/*
 * Tests whether the extraction of XML profiles succeeds when all profiles are wrong.
 * XMLProfileManager::loadXMLNode returns XMLProfileManager::extractProfiles.
 * The expected return value is XMLP_ret::XML_ERROR.
 * Checks an incorrect profile of each type as the only present in the profiles tag
 */
TEST_F(XMLProfileParserBasicTests, extract_profiles_error)
{
    tinyxml2::XMLDocument xml_doc;

    {
        // XML Without a root
        const char* xml = "<a/>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
    }

    {
        const char* xml_p =
                "\
                    <profiles>\
                        <%s>\
                        </%s>\
                    </profiles>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> elements {
            "participant",
            "publisher",
            "subscriber",
            "topic",
            "requester",
            "replier",
        };

        for (const std::string& e : elements)
        {
            snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        // XMLNode with no profile element
        const char* xml = "<log></log>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
    }
}

/*
 * This test check the possible errors in the parsing of the profile of each type and the loading of a profile as
 * the default profile.
 * Errors tested:
 *  1. Empty profile name
 *  2. Loading the same profile twice
 */
TEST_F(XMLProfileParserBasicTests, extract_type_profiles)
{
    tinyxml2::XMLDocument xml_doc;


    {
        // Participant

        const char* xml_p =
                "<profiles>\
            <participant profile_name=\"%s\">\
                <rtps></rtps>\
            </participant>\
        </profiles>";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        // empty name
        snprintf(xml, xml_len, xml_p, "");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

        // same profile name twice
        snprintf(xml, xml_len, xml_p, "test_name");
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

        xmlparser::XMLProfileManager::DeleteInstance();
    }

    {
        // Publisher, Subscriber, and Topic

        const char* xml_p =
                "<profiles>\
            <%s profile_name=\"%s\" %s>\
            </%s>\
        </profiles>";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> elements {
            "publisher",
            "subscriber",
            "topic",
        };

        for (const std::string& e : elements)
        {
            // empty profile name
            snprintf(xml, xml_len, xml_p, e.c_str(), "", "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // default profile
            snprintf(xml, xml_len, xml_p, e.c_str(), "default_name_1", "is_default_profile=\"true\"", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // same profile name twice
            snprintf(xml, xml_len, xml_p, e.c_str(), "test_name", "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        // Requested and Replier

        const char* xml_p =
                "<profiles>\
            <%s profile_name=\"%s\" service_name=\"serv\" request_type=\"req\" reply_type=\"reply\">\
            </%s>\
        </profiles>";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> elements {
            "requester",
            "replier"
        };

        for (const std::string& e : elements)
        {
            // empty profile name
            snprintf(xml, xml_len, xml_p, e.c_str(), "", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));

            // same profile name twice
            snprintf(xml, xml_len, xml_p, e.c_str(), "test_name", e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLNode(xml_doc));
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

}


/*
 * Tests whether the SKIP_DEFAULT_XML_FILE variable prevents the xmlparser from loading the default XML file.
 * participant_atts_none skips the default and obtains the values from the constructors.
 * participant_atts_default contains the attributes in the default file in this folder which should be different.
 */
TEST_F(XMLProfileParserBasicTests, skip_default_xml)
{
    const char* xml =
            "                                                                                                          \
        <profiles>                                                                                                     \
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">                        \
                <domainId>2020268</domainId>                                                                           \
                <rtps></rtps>                                                                                          \
            </participant>                                                                                             \
        </profiles>                                                                                                    \
    ";
    tinyxml2::XMLDocument xml_doc;
    xml_doc.Parse(xml);
    xml_doc.SaveFile("DEFAULT_FASTDDS_PROFILES.xml");

#ifdef _WIN32
    _putenv_s("SKIP_DEFAULT_XML_FILE", "1");
#else
    setenv("SKIP_DEFAULT_XML_FILE", "1", 1);
#endif // ifdef _WIN32
    xmlparser::ParticipantAttributes participant_atts_none;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_none);

#ifdef _WIN32
    _putenv_s("SKIP_DEFAULT_XML_FILE", "");
#else
    unsetenv("SKIP_DEFAULT_XML_FILE");
#endif // ifdef _WIN32
    xmlparser::ParticipantAttributes participant_atts_default;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_default);

    remove("DEFAULT_FASTDDS_PROFILES.xml");

    EXPECT_NE(participant_atts_none.domainId, participant_atts_default.domainId);
}

/*
 * Tests whether the FASTDDS_DEFAULT_PROFILES_FILE environment file correctly loads the selected file as default.
 * - participant_atts_default contains the attributes in the default file in this folder.
 * - participant_atts_file contains the attributes in the default file created by the test.
 */
TEST_F(XMLProfileParserBasicTests, default_env_variable)
{
    const char* xml =
            "                                                                                                                  \
        <profiles>                                                                                                     \
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">                        \
                <domainId>20203011</domainId>                                                                          \
                <rtps></rtps>                                                                                          \
            </participant>                                                                                             \
        </profiles>                                                                                                    \
    ";
    tinyxml2::XMLDocument xml_doc;
    xml_doc.Parse(xml);
    xml_doc.SaveFile("FASTDDS_PROFILES.xml");

    xmlparser::ParticipantAttributes participant_atts_default;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_default);

#ifdef _WIN32
    _putenv_s("FASTDDS_DEFAULT_PROFILES_FILE", "FASTDDS_PROFILES.xml");
#else
    setenv("FASTDDS_DEFAULT_PROFILES_FILE", "FASTDDS_PROFILES.xml", 1);
#endif // ifdef _WIN32
    xmlparser::ParticipantAttributes participant_atts_file;
    xmlparser::XMLProfileManager::loadDefaultXMLFile();
    xmlparser::XMLProfileManager::getDefaultParticipantAttributes(participant_atts_file);
    remove("FASTDDS_PROFILES.xml");

    EXPECT_NE(participant_atts_file.domainId, participant_atts_default.domainId);
}

/*
 * Tests the return codes of the loadXMLFile method.
 *  1. On parsing a correctly parsed standalone profiles file, expect XMLP_ret::XML_OK.
 *  2. On parsing a correctly parsed rooted profiles file, expect XMLP_ret::XML_OK.
 *  3. On parsing an incorrect element inside profiles element, expect XMLP_ret::XML_ERROR.
 *  4. On giving an empty string as a filename, expect XMLP_ret::XML_ERROR.
 *  5. On parsing the same file twice, expect XMLP_ret::XML_OK.
 *  6. On parsing an incorrect element inside profiles element, expect XMLP_ret::XML_ERROR.
 */
TEST_F(XMLProfileParserBasicTests, loadXMLFile)
{
    using namespace eprosima::fastdds::dds;

    const char* filename = "minimal_file.xml";
    tinyxml2::XMLDocument xml_doc;

    EXPECT_CALL(*log_mock, ClearConsumers()).Times(AnyNumber());
    EXPECT_CALL(*log_mock, RegisterConsumer(IsStdoutErrConsumer())).Times(AnyNumber());

    {
        std::vector<std::string> correct_xmls =
        {
            "<profiles>\
            <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">\
                <domainId>20203011</domainId>\
                <rtps></rtps>\
            </participant>\
        </profiles>",
            "<types>\
            <type>\
                <enum name=\"MyAloneEnumType\">\
                    <enumerator name=\"A\" value=\"0\"/>\
                    <enumerator name=\"B\" value=\"1\"/>\
                </enum>\
            </type>\
        </types>",
            "<log>\
            <use_default>FALSE</use_default>\
            <consumer>\
                <class>StdoutErrConsumer</class>\
            </consumer>\
        </log>",
            "<library_settings>\
            <intraprocess_delivery>FULL</intraprocess_delivery>\
        </library_settings>"
        };

        // Standalone
        for (std::string& xml : correct_xmls)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse((xml).c_str()));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }

        // Rooted
        for (std::string& xml : correct_xmls)
        {
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(("<dds>" + xml + "</dds>").c_str()));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        // Negative clauses
        const char* xml_p =
                "\
                <%s>\
                    <bad_tag></bad_tag>\
                </%s>\
                ";
        constexpr size_t xml_len {500};
        char xml[xml_len];

        std::vector<std::string> elements {
            "profiles",
            "types",
            "log",
            "library_settings",
            "bad_profile"
        };

        for (const std::string& e : elements)
        {
            snprintf(xml, xml_len, xml_p, e.c_str(), e.c_str());
            ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
            xml_doc.SaveFile(filename);
            EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLFile(filename));
            remove(filename);
            xmlparser::XMLProfileManager::DeleteInstance();
        }
    }

    {
        const char* xml =
                "<profiles>\
                <participant profile_name=\"test_participant_profile\" is_default_profile=\"true\">\
                    <domainId>20203011</domainId>\
                    <rtps></rtps>\
                </participant>\
            </profiles>";
        ASSERT_EQ(tinyxml2::XMLError::XML_SUCCESS, xml_doc.Parse(xml));
        xml_doc.SaveFile(filename);

        // passing an empty string as a filename
        EXPECT_EQ(xmlparser::XMLP_ret::XML_ERROR, xmlparser::XMLProfileManager::loadXMLFile(""));

        // Parsing same file twice
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
        EXPECT_EQ(xmlparser::XMLP_ret::XML_OK, xmlparser::XMLProfileManager::loadXMLFile(filename));
        remove(filename);
        xmlparser::XMLProfileManager::DeleteInstance();
    }

}

/**
 * This test checks positive and negative cases for parsing of external locators related configuration
 */
TEST_F(XMLProfileParserBasicTests, external_locators_feature)
{
    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
    };

    std::vector<TestCase> test_cases =
    {
        // ------------------ participant positive cases
        {
            "participant_no_external",
            R"(<profiles><participant profile_name="p1"><rtps/></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_default_empty",
            R"(<profiles><participant profile_name="p1"><rtps>
                <default_external_unicast_locators>
                </default_external_unicast_locators>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_default_udp4",
            R"(<profiles><participant profile_name="p1"><rtps>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </default_external_unicast_locators>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_default_udp6",
            R"(<profiles><participant profile_name="p1"><rtps>
                <default_external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1234</port>
                    </udpv6>
                </default_external_unicast_locators>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_meta_empty",
            R"(<profiles><participant profile_name="p1"><rtps><builtin>
                <metatraffic_external_unicast_locators>
                </metatraffic_external_unicast_locators>
            </builtin></rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_meta_udp4",
            R"(<profiles><participant profile_name="p1"><rtps><builtin>
                <metatraffic_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </metatraffic_external_unicast_locators>
            </builtin></rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_external_meta_udp6",
            R"(<profiles><participant profile_name="p1"><rtps><builtin>
                <metatraffic_external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1234</port>
                    </udpv6>
                </metatraffic_external_unicast_locators>
            </builtin></rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_complete_ignore_true",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "participant_complete_ignore_false",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        // ------------------ participant negative cases
        {
            "participant_complete_ignore_empty",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators></ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_mask_0",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="0">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_mask_256",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="256">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_mask_negative",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="-100">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_cost_negative",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="-100" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_cost_256",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="256" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_ext_0",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="0" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_duplicated_meta",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                    </metatraffic_external_unicast_locators>
                    <metatraffic_external_unicast_locators>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_duplicated_default",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </default_external_unicast_locators>
                <default_external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_duplicated_ignore",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_ext_256",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="256" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_udp4_multicast",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>239.255.0.1</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_udp6_multicast",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>ff1e::ffff:efff:1</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_empty",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_no_addr",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_addr_duplicated",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <address>192.168.1.101</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_no_port",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_port_duplicated",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1233</port>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_tcp4",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <tcpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </tcpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "participant_complete_wrong_loc_tcp6",
            R"(<profiles><participant profile_name="p1"><rtps>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <default_external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <tcpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </tcpv6>
                </default_external_unicast_locators>
                <builtin>
                    <metatraffic_external_unicast_locators>
                        <udpv4 externality="1" cost="0" mask="24">
                            <address>192.168.1.100</address>
                            <port>2234</port>
                        </udpv4>
                        <udpv6 externality="1" cost="0" mask="24">
                            <address>::1:2</address>
                            <port>2235</port>
                        </udpv6>
                    </metatraffic_external_unicast_locators>
                </builtin>
            </rtps></participant></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        // ------------------ data_writer positive cases
        {
            "writer_no_external",
            R"(<profiles><data_writer profile_name="w1"></data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "writer_external_empty",
            R"(<profiles><data_writer profile_name="w1">
                <external_unicast_locators>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "writer_external_udp4",
            R"(<profiles><data_writer profile_name="w1">
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "writer_external_udp6",
            R"(<profiles><data_writer profile_name="w1">
                <external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1234</port>
                    </udpv6>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "writer_external_complete_ignore_true",
            R"(<profiles><data_writer profile_name="w1">
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "writer_external_complete_ignore_false",
            R"(<profiles><data_writer profile_name="w1">
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        // ------------------ data_writer negative cases
        {
            "writer_external_complete_duplicated_ignore",
            R"(<profiles><data_writer profile_name="w1">
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "writer_external_complete_duplicated_external",
            R"(<profiles><data_writer profile_name="w1">
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </external_unicast_locators>
                <external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_writer></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        // ------------------ data_reader positive cases
        {
            "reader_no_external",
            R"(<profiles><data_reader profile_name="r1"></data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "reader_external_empty",
            R"(<profiles><data_reader profile_name="r1">
                <external_unicast_locators>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "reader_external_udp4",
            R"(<profiles><data_reader profile_name="r1">
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "reader_external_udp6",
            R"(<profiles><data_reader profile_name="r1">
                <external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1234</port>
                    </udpv6>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "reader_external_complete_ignore_true",
            R"(<profiles><data_reader profile_name="r1">
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "reader_external_complete_ignore_false",
            R"(<profiles><data_reader profile_name="r1">
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_OK
        },
        // ------------------ data_reader negative cases
        {
            "reader_external_complete_duplicated_ignore",
            R"(<profiles><data_reader profile_name="r1">
                <ignore_non_matching_locators>true</ignore_non_matching_locators>
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "reader_external_complete_duplicated_external",
            R"(<profiles><data_reader profile_name="r1">
                <ignore_non_matching_locators>false</ignore_non_matching_locators>
                <external_unicast_locators>
                    <udpv4 externality="1" cost="0" mask="24">
                        <address>192.168.1.100</address>
                        <port>1234</port>
                    </udpv4>
                </external_unicast_locators>
                <external_unicast_locators>
                    <udpv6 externality="1" cost="0" mask="24">
                        <address>::1:2</address>
                        <port>1235</port>
                    </udpv6>
                </external_unicast_locators>
            </data_reader></profiles>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
    };

    for (const TestCase& test : test_cases)
    {
        EXPECT_EQ(test.result, xmlparser::XMLProfileManager::loadXMLString(test.xml.c_str(), test.xml.length())) <<
            " test_case = [" << test.title << "]";
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/**
 * This test checks positive and negative cases for parsing of ThreadSettings in Log
 */
TEST_F(XMLProfileParserBasicTests, log_thread_settings_qos)
{
    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
    };

    std::vector<TestCase> test_cases =
    {
        /* Log test cases */
        {
            "log_thread_settings_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <log>
                        <thread_settings>
                            <scheduling_policy>-1</scheduling_policy>
                            <priority>0</priority>
                            <affinity>0</affinity>
                            <stack_size>-1</stack_size>
                        </thread_settings>
                    </log>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK
        },
        {
            "log_thread_settings_duplicate",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <log>
                        <thread_settings>
                            <scheduling_policy>-1</scheduling_policy>
                            <priority>0</priority>
                            <affinity>0</affinity>
                            <stack_size>-1</stack_size>
                        </thread_settings>
                        <thread_settings>
                            <scheduling_policy>-1</scheduling_policy>
                            <priority>0</priority>
                            <affinity>0</affinity>
                            <stack_size>-1</stack_size>
                        </thread_settings>
                    </log>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
        {
            "log_thread_settings_wrong",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <log>
                        <thread_settings>
                            <scheduling_policy>-1</scheduling_policy>
                            <priority>0</priority>
                            <affinity>0</affinity>
                            <stack_size>-1</stack_size>
                            <wrong_tag>0</wrong_tag>
                        </thread_settings>
                    </log>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR
        },
    };

    EXPECT_CALL(*log_mock, SetThreadConfig()).Times(1);
    for (const TestCase& test : test_cases)
    {
        EXPECT_EQ(test.result, xmlparser::XMLProfileManager::loadXMLString(test.xml.c_str(), test.xml.length())) <<
            " test_case = [" << test.title << "]";
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/**
 * This test checks positive and negative cases for parsing of DomainParticipantFactory Qos
 */
TEST_F(XMLProfileParserBasicTests, domainparticipantfactory)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;
    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
        EntityFactoryQosPolicy entity_factory;
        ThreadSettings shm_watchdog_thread;
        ThreadSettings file_watch_threads;
    };

    ThreadSettings default_thread_settings;
    ThreadSettings modified_thread_settings;
    modified_thread_settings.scheduling_policy = 12;
    modified_thread_settings.priority = 12;
    modified_thread_settings.affinity = 12;
    modified_thread_settings.stack_size = 12;

    std::vector<TestCase> test_cases =
    {
        /* DomainParticipantFactory cases */
        {
            "entity_factory_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            EntityFactoryQosPolicy(false),
            default_thread_settings,
            default_thread_settings
        },
        {
            "shm_watchdog_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            EntityFactoryQosPolicy(true),
            modified_thread_settings,
            default_thread_settings
        },
        {
            "file_watch_threads_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            EntityFactoryQosPolicy(true),
            default_thread_settings,
            modified_thread_settings
        },
        {
            "all_present_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            EntityFactoryQosPolicy(false),
            modified_thread_settings,
            modified_thread_settings
        },
        {
            "qos_duplicated",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                        </qos>
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            default_thread_settings,
            default_thread_settings
        },
        {
            "entity_factory_wrong_tag",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <wrong_tag>false</wrong_tag>
                            </entity_factory>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            default_thread_settings,
            default_thread_settings
        },
        {
            "entity_factory_duplicated_autoenable_created_entities_tag",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            default_thread_settings,
            default_thread_settings
        },
        {
            "entity_factory_duplicated_autoenable_created_entities_and_wrong_tag",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                                <wrong_tag>false</wrong_tag>
                            </entity_factory>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            default_thread_settings,
            default_thread_settings
        },
        {
            "entity_factory_duplicated",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            modified_thread_settings,
            modified_thread_settings
        },
        {
            "shm_watchdog_thread_duplicated",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            modified_thread_settings,
            modified_thread_settings
        },
        {
            "file_watch_threads_duplicated",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <domainparticipant_factory profile_name="factory" is_default_profile="true">
                        <qos>
                            <entity_factory>
                                <autoenable_created_entities>false</autoenable_created_entities>
                            </entity_factory>
                            <shm_watchdog_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </shm_watchdog_thread>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                            <file_watch_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </file_watch_threads>
                        </qos>
                        </domainparticipant_factory>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            EntityFactoryQosPolicy(true),
            modified_thread_settings,
            modified_thread_settings
        }
    };

    for (const TestCase& test : test_cases)
    {
        EXPECT_EQ(test.result, xmlparser::XMLProfileManager::loadXMLString(test.xml.c_str(), test.xml.length())) <<
            " test_case = [" << test.title << "]";
        if (test.result == xmlparser::XMLP_ret::XML_OK)
        {
            using namespace eprosima::fastdds::dds;

            DomainParticipantFactoryQos profile_qos;
            ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
                    xmlparser::XMLProfileManager::fillDomainParticipantFactoryQos("factory", profile_qos));

            DomainParticipantFactoryQos default_qos;
            xmlparser::XMLProfileManager::getDefaultDomainParticipantFactoryQos(default_qos);

            ASSERT_EQ(profile_qos, default_qos);
            ASSERT_EQ(profile_qos.entity_factory(), test.entity_factory);
            ASSERT_EQ(profile_qos.shm_watchdog_thread(), test.shm_watchdog_thread);
            ASSERT_EQ(profile_qos.file_watch_threads(), test.file_watch_threads);
        }
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/**
 * This test checks positive and negative cases for parsing of DomainParticipant ThreadSettings
 */
TEST_F(XMLProfileParserBasicTests, participant_thread_settings)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;
    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
        ThreadSettings builtin_controllers_sender_thread;
        ThreadSettings timed_events_thread;
        ThreadSettings discovery_server_thread;
        ThreadSettings builtin_transports_reception_threads;
#if HAVE_SECURITY
        ThreadSettings security_log_thread;
#endif // if HAVE_SECURITY
    };

    ThreadSettings default_thread_settings;
    ThreadSettings modified_thread_settings;
    modified_thread_settings.scheduling_policy = 12;
    modified_thread_settings.priority = 12;
    modified_thread_settings.affinity = 12;
    modified_thread_settings.stack_size = 12;

    std::vector<TestCase> test_cases =
    {
        {
            "builtin_controllers_sender_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <builtin_controllers_sender_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </builtin_controllers_sender_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            modified_thread_settings,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "builtin_controllers_sender_thread_nok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <builtin_controllers_sender_thread>
                                <wrong>12</wrong>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </builtin_controllers_sender_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            modified_thread_settings,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "timed_events_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <timed_events_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </timed_events_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
            modified_thread_settings,
            default_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "timed_events_thread_nok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <timed_events_thread>
                                <wrong>12</wrong>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </timed_events_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
            modified_thread_settings,
            default_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "discovery_server_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <discovery_server_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </discovery_server_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "discovery_server_thread_nok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <discovery_server_thread>
                                <wrong>12</wrong>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </discovery_server_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings,
            default_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "builtin_transports_reception_threads_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <builtin_transports_reception_threads>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </builtin_transports_reception_threads>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
        {
            "builtin_transports_reception_threads_nok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <builtin_transports_reception_threads>
                                <wrong>12</wrong>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </builtin_transports_reception_threads>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings,
#if HAVE_SECURITY
            default_thread_settings
#endif // if HAVE_SECURITY
        },
#if HAVE_SECURITY
        {
            "security_log_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <security_log_thread>
                                <scheduling_policy>12</scheduling_policy>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </security_log_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings
        },
        {
            "security_log_thread_nok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <participant profile_name="participant" is_default_profile="true">
                        <rtps>
                            <security_log_thread>
                                <wrong>12</wrong>
                                <priority>12</priority>
                                <affinity>12</affinity>
                                <stack_size>12</stack_size>
                            </security_log_thread>
                        </rtps>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            default_thread_settings,
            modified_thread_settings
        },
#endif // if HAVE_SECURITY
    };

    for (const TestCase& test : test_cases)
    {
        EXPECT_EQ(test.result, xmlparser::XMLProfileManager::loadXMLString(test.xml.c_str(), test.xml.length())) <<
            " test_case = [" << test.title << "]";
        if (test.result == xmlparser::XMLP_ret::XML_OK)
        {
            using namespace eprosima::fastdds::dds;

            xmlparser::ParticipantAttributes profile_attr;
            ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
                    xmlparser::XMLProfileManager::fillParticipantAttributes("participant", profile_attr));

            xmlparser::ParticipantAttributes default_attr;
            xmlparser::XMLProfileManager::getDefaultParticipantAttributes(default_attr);

            ASSERT_EQ(profile_attr, default_attr);
            ASSERT_EQ(profile_attr.rtps.builtin_controllers_sender_thread, test.builtin_controllers_sender_thread);
            ASSERT_EQ(profile_attr.rtps.timed_events_thread, test.timed_events_thread);
            ASSERT_EQ(profile_attr.rtps.discovery_server_thread, test.discovery_server_thread);
            ASSERT_EQ(profile_attr.rtps.builtin_transports_reception_threads,
                    test.builtin_transports_reception_threads);
#if HAVE_SECURITY
            ASSERT_EQ(profile_attr.rtps.security_log_thread, test.security_log_thread);
#endif // if HAVE_SECURITY
        }
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

/**
 * This test checks positive and negative cases for parsing of DataReader ThreadSettings
 */
TEST_F(XMLProfileParserBasicTests, datareader_thread_settings)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;
    struct TestCase
    {
        std::string title;
        std::string xml;
        xmlparser::XMLP_ret result;
        ThreadSettings data_sharing_listener_thread;
    };

    ThreadSettings default_thread_settings;
    ThreadSettings modified_thread_settings;
    modified_thread_settings.scheduling_policy = 12;
    modified_thread_settings.priority = 12;
    modified_thread_settings.affinity = 12;
    modified_thread_settings.stack_size = 12;

    std::vector<TestCase> test_cases =
    {
        {
            "data_sharing_listener_thread_ok",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <data_reader profile_name="datareader" is_default_profile="true">
                            <qos>
                                <data_sharing>
                                    <kind>AUTOMATIC</kind>
                                    <data_sharing_listener_thread>
                                        <scheduling_policy>12</scheduling_policy>
                                        <priority>12</priority>
                                        <affinity>12</affinity>
                                        <stack_size>12</stack_size>
                                    </data_sharing_listener_thread>
                                </data_sharing>
                            </qos>
                        </data_reader>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            modified_thread_settings
        },
        {
            "data_sharing_listener_thread_empty",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <data_reader profile_name="datareader" is_default_profile="true">
                            <qos>
                                <data_sharing>
                                    <kind>AUTOMATIC</kind>
                                    <data_sharing_listener_thread>
                                    </data_sharing_listener_thread>
                                </data_sharing>
                            </qos>
                        </data_reader>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
        },
        {
            "no_data_sharing_listener_thread",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <data_reader profile_name="datareader" is_default_profile="true">
                            <qos>
                                <data_sharing>
                                    <kind>AUTOMATIC</kind>
                                </data_sharing>
                            </qos>
                        </data_reader>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_OK,
            default_thread_settings,
        },
        {
            "data_sharing_listener_thread_wrong_value",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <data_reader profile_name="datareader" is_default_profile="true">
                            <qos>
                                <data_sharing>
                                    <kind>AUTOMATIC</kind>
                                    <data_sharing_listener_thread>
                                        <scheduling_policy>aa</scheduling_policy>
                                        <priority>0</priority>
                                        <affinity>0</affinity>
                                        <stack_size>0</stack_size>
                                    </data_sharing_listener_thread>
                                </data_sharing>
                            </qos>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
        },
        {
            "data_sharing_listener_thread_wrong_tag",
            R"(
                <?xml version="1.0" encoding="UTF-8" ?>
                <dds xmlns="http://www.eprosima.com">
                    <profiles>
                        <data_reader profile_name="datareader" is_default_profile="true">
                            <qos>
                                <data_sharing>
                                    <kind>AUTOMATIC</kind>
                                    <data_sharing_listener_thread>
                                        <wrong>-1</wrong>
                                        <priority>0</priority>
                                        <affinity>0</affinity>
                                        <stack_size>0</stack_size>
                                    </data_sharing_listener_thread>
                                </data_sharing>
                            </qos>
                        </participant>
                    </profiles>
                </dds>)",
            xmlparser::XMLP_ret::XML_ERROR,
            default_thread_settings,
        },
    };

    for (const TestCase& test : test_cases)
    {
        EXPECT_EQ(test.result, xmlparser::XMLProfileManager::loadXMLString(test.xml.c_str(), test.xml.length())) <<
            " test_case = [" << test.title << "]";
        if (test.result == xmlparser::XMLP_ret::XML_OK)
        {
            xmlparser::SubscriberAttributes profile_attr;
            ASSERT_EQ(xmlparser::XMLP_ret::XML_OK,
                    xmlparser::XMLProfileManager::fillSubscriberAttributes("datareader", profile_attr));

            xmlparser::SubscriberAttributes default_attr;
            xmlparser::XMLProfileManager::getDefaultSubscriberAttributes(default_attr);

            ASSERT_EQ(profile_attr, default_attr);
            ASSERT_EQ(profile_attr.qos.data_sharing.data_sharing_listener_thread(), test.data_sharing_listener_thread);
        }
        xmlparser::XMLProfileManager::DeleteInstance();
    }
}

INSTANTIATE_TEST_SUITE_P(XMLProfileParserTests, XMLProfileParserTests, testing::Values(false));
INSTANTIATE_TEST_SUITE_P(XMLProfileParserEnvVarTests, XMLProfileParserTests, testing::Values(true));

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
