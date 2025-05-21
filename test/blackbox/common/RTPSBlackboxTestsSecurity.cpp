// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#if HAVE_SECURITY

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;


void set_authentication_config(
        rtps::PropertySeq& properties)
{
    properties.emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");
    properties.emplace_back("dds.sec.auth.builtin.PKI-DH.identity_ca",
            "file://" + std::string(certs_path) + "/maincacert.pem");
    properties.emplace_back("dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file://" + std::string(certs_path) + "/mainpubcert.pem");
    properties.emplace_back("dds.sec.auth.builtin.PKI-DH.private_key",
            "file://" + std::string(certs_path) + "/mainpubkey.pem");
}

void set_participant_crypto_config(
        rtps::PropertySeq& props,
        const std::string& governance_file = "governance_helloworld_all_enable.smime",
        const std::string& permissions_file = "permissions_helloworld.smime")
{
    props.emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
    props.emplace_back(Property("dds.sec.access.plugin",
            "builtin.Access-Permissions"));
    props.emplace_back(Property(
                "dds.sec.access.builtin.Access-Permissions.permissions_ca",
                "file://" + std::string(certs_path) + "/maincacert.pem"));
    props.emplace_back(Property("dds.sec.access.builtin.Access-Permissions.governance",
            "file://" + std::string(certs_path) + "/" + governance_file));
    props.emplace_back(Property("dds.sec.access.builtin.Access-Permissions.permissions",
            "file://" + std::string(certs_path) + "/" + permissions_file));
}

/**
 * This test checks that a StatefulWriter created with security,
 * but with a wrong permissions on topic, is properly cleaned up
 */
TEST(RTPSSecurityTests, statefulwriter_wrong_permissions_properly_cleaned_up)
{
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer("CustomTopicName");
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader("CustomTopicName");

    PropertyPolicy security_properties;
    set_authentication_config(security_properties.properties());
    set_participant_crypto_config(security_properties.properties());

    writer.add_participant_properties(security_properties);
    reader.add_participant_properties(security_properties);

    // This should fail but properly exit
    ASSERT_NO_THROW(writer.init());
    ASSERT_NO_THROW(reader.init());
}

void blackbox_security_init()
{
    certs_path = std::getenv("CERTS_PATH");

    if (certs_path == nullptr)
    {
        std::cout << "Cannot get enviroment variable CERTS_PATH" << std::endl;
        exit(-1);
    }
}

#endif // if HAVE_SECURITY

