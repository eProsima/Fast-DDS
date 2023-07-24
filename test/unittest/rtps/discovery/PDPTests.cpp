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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <rtps/participant/RTPSParticipantImpl.h>

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

class PDPTests : public ::testing::Test
{

protected:

    void SetUp() override
    {
        //BuiltinProtocols bp;
        RTPSParticipantAllocationAttributes attrs;
        //bp.initBuiltinProtocols(participant_, attrs);

        //pdp = new PDPMock(&bp, &attrs);
    }

    void TearDown() override
    {
        delete pdp;
    }

    void set_incompatible_topic()
    {
        //rdata->topicName("AnotherTopic");
    }

    void set_incompatible_topic_kind()
    {
        //rdata->topicKind(TopicKind_t::WITH_KEY);
    }

    void set_incompatible_type()
    {
        //rdata->typeName("AnotherTypeName");
    }

    //::testing::NiceMock<BuiltinProtocols> builtin_prot_;
    //::testing::NiceMock<RTPSParticipantImpl> participant_;
    //::testing::NiceMock<WriterProxyData>* wdata;
    //::testing::NiceMock<ReaderProxyData>* rdata;
    PDPMock* pdp;
};

TEST(PDPTests, Sample)
{
    //foo
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
