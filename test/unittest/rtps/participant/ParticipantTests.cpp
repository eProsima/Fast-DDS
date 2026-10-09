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

#include <memory>
#include <string>
#include <vector>

#include <asio.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <MockConsumer.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace testing;

RTPSParticipant* transport_size_participant_init(
        uint32_t max_message_size)
{
    uint32_t domain_id = 0;
    std::string max_message_size_str = std::to_string(max_message_size);

    RTPSParticipantAttributes p_attr;
    BuiltinTransportsOptions options;
    options.maxMessageSize = max_message_size;
    p_attr.setup_transports(BuiltinTransports::SHM, options);
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    return participant;
}

/**
 * This test checks that the participant is not created when the max message size is smaller than the PDP package size
 * but it is properly created when the max message size is bigger than the PDP package size.
 */
TEST(RTPSParticipantTests, participant_creation_message_size)
{
    ASSERT_EQ(transport_size_participant_init(100), nullptr);
    ASSERT_NE(transport_size_participant_init(1000), nullptr);
}

/**
 * Regression tests for GitHub issue #6528: successful receiver locator mutations must be visible at Warning level.
 */
#if !HAVE_LOG_NO_WARNING
class ReceiverPortMutationTests : public TestWithParam<uint32_t>
{
protected:

    void SetUp() override
    {
        using asio::ip::udp;
        const auto loopback = asio::ip::address_v4::loopback();

        // Reserve a free range without relying on fixed ports or the participant ID allocated by other tests.
        for (uint32_t attempt = 0; attempt < 100; ++attempt)
        {
            reserved_ports_.clear();
            reserved_ports_.emplace_back(new udp::socket(io_context_, udp::endpoint(loopback, 0)));
            original_locator_.kind = LOCATOR_KIND_UDPv4;
            original_locator_.port = reserved_ports_.front()->local_endpoint().port();
            IPLocator::setIPv4(original_locator_, "127.0.0.1");

            if (original_locator_.port > 65529)
            {
                continue;
            }

            for (uint32_t i = 1; i < 4; ++i)
            {
                std::unique_ptr<udp::socket> socket(new udp::socket(io_context_, udp::v4()));
                asio::error_code error;
                socket->bind(udp::endpoint(loopback, static_cast<uint16_t>(original_locator_.port + 2 * i)), error);
                if (error)
                {
                    break;
                }
                reserved_ports_.push_back(std::move(socket));
            }
            if (reserved_ports_.size() == 4)
            {
                break;
            }
        }
        ASSERT_EQ(4u, reserved_ports_.size());

        dds::Log::Flush();
        dds::Log::Reset();
        dds::Log::ClearConsumers();
        consumer_ = new dds::MockConsumer("RTPS_PARTICIPANT");
        dds::Log::RegisterConsumer(std::unique_ptr<dds::LogConsumer>(consumer_));
        dds::Log::SetVerbosity(dds::Log::Warning);
        dds::Log::ReportFunctions(true);
    }

    void TearDown() override
    {
        if (participant_ != nullptr)
        {
            RTPSDomain::removeRTPSParticipant(participant_);
        }
        dds::Log::Flush();
        dds::Log::Reset();
    }

    void create_participant(
            uint32_t occupied_ports,
            uint32_t mutation_tries)
    {
        reserved_ports_.resize(occupied_ports);
        RTPSParticipantAttributes attributes;
        attributes.useBuiltinTransports = false;
        auto transport = std::make_shared<UDPv4TransportDescriptor>();
        transport->interface_allowlist.emplace_back("127.0.0.1");
        attributes.userTransports.push_back(transport);
        attributes.builtin.metatrafficUnicastLocatorList.push_back(original_locator_);
        Locator_t user_locator = original_locator_;
        user_locator.port += 6;
        attributes.defaultUnicastLocatorList.push_back(user_locator);
        attributes.builtin.mutation_tries = mutation_tries;
        // Binding happens during construction; no discovery traffic is needed for these tests.
        participant_ = RTPSDomain::createParticipant(0, false, attributes);
        dds::Log::Flush();
    }

    std::vector<dds::Log::Entry> receiver_warnings() const
    {
        std::vector<dds::Log::Entry> result;
        for (const auto& entry : consumer_->ConsumedEntries())
        {
            if (entry.kind == dds::Log::Warning && entry.context.function != nullptr &&
                    std::string(entry.context.function).find("createReceiverResources") != std::string::npos)
            {
                result.push_back(entry);
            }
        }
        return result;
    }

    asio::io_context io_context_;
    std::vector<std::unique_ptr<asio::ip::udp::socket>> reserved_ports_;
    Locator_t original_locator_;
    RTPSParticipant* participant_ = nullptr;
    dds::MockConsumer* consumer_ = nullptr;
};

TEST_P(ReceiverPortMutationTests, successful_binding_reports_only_actual_mutations)
{
    const uint32_t occupied_ports = GetParam();
    create_participant(occupied_ports, 3);
    ASSERT_NE(nullptr, participant_);

    const auto attributes = participant_->copy_attributes();
    ASSERT_EQ(1u, attributes.builtin.metatrafficUnicastLocatorList.size());
    const auto& bound_locator = *attributes.builtin.metatrafficUnicastLocatorList.begin();
    EXPECT_EQ(original_locator_.port + 2 * occupied_ports, bound_locator.port);

    const auto warnings = receiver_warnings();
    if (occupied_ports == 0)
    {
        EXPECT_TRUE(warnings.empty());
    }
    else
    {
        // Multiple failed attempts produce one actionable warning containing the requested and actual locators.
        ASSERT_EQ(1u, warnings.size());
        EXPECT_NE(std::string::npos, warnings.front().message.find(IPLocator::to_string(original_locator_)));
        EXPECT_NE(std::string::npos, warnings.front().message.find(IPLocator::to_string(bound_locator)));
    }
}

TEST_F(ReceiverPortMutationTests, exhausted_mutations_keep_the_failure_warning)
{
    create_participant(2, 1);
    if (participant_ != nullptr)
    {
        EXPECT_TRUE(participant_->copy_attributes().builtin.metatrafficUnicastLocatorList.empty());
    }

    const auto warnings = receiver_warnings();
    ASSERT_EQ(1u, warnings.size());
    EXPECT_NE(std::string::npos, warnings.front().message.find("Could not create the specified receiver resource"));
}

INSTANTIATE_TEST_SUITE_P(OccupiedPorts, ReceiverPortMutationTests, Values(0u, 1u, 2u));
#endif // if !HAVE_LOG_NO_WARNING

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
