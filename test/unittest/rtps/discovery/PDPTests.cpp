// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <iostream>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

#if defined(__cplusplus_winrt)
#define GET_PID GetCurrentProcessId
#elif defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

namespace eprosima {
namespace fastrtps {
namespace rtps {

using ::testing::Return;
using ::testing::ReturnRef;

class PDPMock : public PDP
{
public:

    PDPMock(
            BuiltinProtocols* bp,
            const RTPSParticipantAllocationAttributes& allocs)
        : PDP(bp, allocs)
    {
    }

    virtual ~PDPMock() // = default;
    {
    }

    void initializeParticipantProxyData(
            ParticipantProxyData* /*participant_data*/) override
    {
        return;
    }

    bool init(
            RTPSParticipantImpl* /*part*/) override
    {
        return true;
    }

    ParticipantProxyData* createParticipantProxyData(
            const ParticipantProxyData& /*p*/,
            const GUID_t& /*writer_guid*/) override
    {
        return nullptr;
    }

    void announceParticipantState(
            bool /*new_change*/,
            bool /*dispose*/,
            WriteParams& /*wparams*/) override
    {
        return;
    }

    void announceParticipantState(
            bool /*new_change*/,
            bool /*dispose = false*/) override
    {
        return;
    }

    void stopParticipantAnnouncement() override
    {
        return;
    }

    void resetParticipantAnnouncement() override
    {
        return;
    }

    bool createPDPEndpoints() override
    {
        return true;
    }

    void assignRemoteEndpoints(
            ParticipantProxyData* /*pdata*/) override
    {
        return;
    }

    void notifyAboveRemoteEndpoints(
            const ParticipantProxyData& /*pdata*/,
            bool /*notify_secure_endpoints*/) override
    {
        return;
    }

    bool updateInfoMatchesEDP() override
    {
        return true;
    }

    void removeRemoteEndpoints(
            ParticipantProxyData* /*pdata*/) override
    {
        return;
    }

    bool remove_remote_participant(
            const GUID_t& /*participant_guid*/,
            ParticipantDiscoveryInfo::DISCOVERY_STATUS /*reason*/) override
    {
        return true;
    }

#if HAVE_SECURITY
    bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& /*local_reader*/,
            const WriterProxyData& /*remote_writer_data*/) override
    {
        return true;
    }

    bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& /*local_writer*/,
            const ReaderProxyData& /*remote_reader_data*/) override
    {
        return true;
    }

#endif // HAVE_SECURITY

protected:

    void update_builtin_locators() override
    {
    }

};


class Listener : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    Listener()
        : matched(0)
    {
    }

    ~Listener() override
    {
    }

    void on_participant_discovery(
            fastdds::dds::DomainParticipant* participant,
            fastrtps::rtps::ParticipantDiscoveryInfo&& /*info*/) override
    {
        if (std::find(p_matched_.begin(), p_matched_.end(), participant->guid()) == p_matched_.end())
        {
            matched++;
            p_matched_.push_back(participant->guid());
        }
    }

    void on_participant_discovery(
            fastdds::dds::DomainParticipant* participant,
            fastrtps::rtps::ParticipantDiscoveryInfo&& /*info*/,
            bool& /*should_be_ignored*/) override
    {
        if (std::find(p_matched_.begin(), p_matched_.end(), participant->guid()) == p_matched_.end())
        {
            matched++;
            p_matched_.push_back(participant->guid());
        }
    }

    int matched;

private:

    std::vector<fastrtps::rtps::GUID_t> p_matched_;
};

class Participant
{

public:

    Participant()
        : listener(new Listener())
    {
    }

    ~Participant() = default;

    void setup()
    {
        fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
            (uint32_t)GET_PID() % 230, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT, listener);
    }

    Listener* listener;
};

class PDPTests : public ::testing::Test
{

protected:

    void SetUp() override
    {
        const RTPSParticipantAllocationAttributes attrs;
        pdp = new PDPMock(nullptr, attrs);
    }

    void TearDown() override
    {
        delete pdp;
    }

    PDPMock* pdp;
};

TEST(PDPTests, ParticipantsMatch)
{
    Participant* p1 = new Participant();
    ASSERT_EQ(p1->listener->matched, 0);

    Participant* p2 = new Participant();
    ASSERT_EQ(p2->listener->matched, 0);

    p1->setup();
    p2->setup();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ASSERT_EQ(p1->listener->matched, 1);
    ASSERT_EQ(p2->listener->matched, 1);
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
