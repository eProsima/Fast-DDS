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
#include <future>
#include <iostream>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <statistics/fastdds/domain/DomainParticipantImpl.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

#if HAVE_SECURITY
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY

#if defined(__cplusplus_winrt)
#define GET_PID GetCurrentProcessId
#elif defined(_WIN32)
#include <process.h>
#define GET_PID _getpid
#else
#define GET_PID getpid
#endif // if defined(_WIN32)

namespace eprosima {
namespace fastdds {
namespace rtps {

using ::testing::Return;
using ::testing::ReturnRef;

class TesterPDPEndpoints : public fastdds::rtps::PDPEndpoints
{
    ~TesterPDPEndpoints() override = default;

    fastdds::rtps::BuiltinEndpointSet_t builtin_endpoints() const override
    {
        return fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
               fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    }

    const std::unique_ptr<fastdds::rtps::ReaderListener>& main_listener() const override
    {
        return no_listener_;
    }

    bool enable_pdp_readers(
            fastdds::rtps::RTPSParticipantImpl*) override
    {
        return true;
    }

    void disable_pdp_readers(
            fastdds::rtps::RTPSParticipantImpl*) override
    {

    }

    void delete_pdp_endpoints(
            fastdds::rtps::RTPSParticipantImpl* ) override
    {

    }

    void remove_from_pdp_reader_history(
            const fastdds::rtps::InstanceHandle_t&) override
    {

    }

    void remove_from_pdp_reader_history(
            fastdds::rtps::CacheChange_t*) override
    {

    }

    std::unique_ptr<fastdds::rtps::ReaderListener> no_listener_;

};

class PDPTester : public PDP
{
public:

    PDPTester(
            BuiltinProtocols* bp,
            const RTPSParticipantAllocationAttributes& allocs)
        : PDP(bp, allocs)
        , endpoints_(nullptr)
    {
        endpoints_ = new TesterPDPEndpoints();
        builtin_endpoints_.reset(endpoints_);
    }

    virtual ~PDPTester()
    {
        for (size_t i = 0; i < pdatas_.size(); i++)
        {
            delete pdatas_[i];
        }
    }

    bool init(
            RTPSParticipantImpl* part) override
    {
        mp_RTPSParticipant = part;
        return true;
    }

    ParticipantProxyData* createParticipantProxyData(
            const ParticipantProxyData& /*p*/,
            const GUID_t& /*writer_guid*/) override
    {
        return nullptr;
    }

    void create_and_add_participant_proxy_data(
            const GUID_t& part_guid)
    {
        RTPSParticipantAllocationAttributes attrs;
        ParticipantProxyData* pdata = new ParticipantProxyData(attrs);
        pdata->guid = part_guid;

        add_participant_proxy_data(part_guid, false, pdata);
        pdatas_.push_back(pdata);
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
            ParticipantDiscoveryStatus /*reason*/) override
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

    TesterPDPEndpoints* endpoints_;
    std::vector<ParticipantProxyData*> pdatas_;

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
            fastdds::rtps::ParticipantDiscoveryStatus /*status*/,
            const fastdds::rtps::ParticipantBuiltinTopicData& /*info*/,
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

    std::vector<fastdds::rtps::GUID_t> p_matched_;
};

class PDPTests : public ::testing::Test
{

protected:

    void SetUp() override
    {
        RTPSParticipantAllocationAttributes attrs;
        attrs.participants.initial = 3;
        attrs.readers.initial = 3;
        attrs.writers.initial = 3;
        pdp_ = new PDPTester(&bp_, attrs);

        pdp_->init(&participant_);
    }

    void TearDown() override
    {
        delete pdp_;
    }

    PDPTester* pdp_;
    ::testing::NiceMock<BuiltinProtocols> bp_;
    ::testing::NiceMock<RTPSParticipantImpl> participant_;
};

TEST_F(PDPTests, iproxy_queryable_get_all_local_proxies)
{
#ifdef FASTDDS_STATISTICS

    std::vector<GUID_t> local_guids, output_guids;
    size_t n_entities = 10;
    local_guids.reserve(n_entities);

    GUID_t part_guid(GuidPrefix_t::unknown(), ENTITYID_RTPSParticipant);
    EXPECT_CALL(participant_, getGuid()).WillRepeatedly(testing::ReturnRef(part_guid));
    pdp_->create_and_add_participant_proxy_data(part_guid);
    local_guids.push_back(part_guid);

    //! Generate local entities
    for (size_t i = 1; i < 10; i++)
    {
        EntityId_t entity;
        entity.value[3] = (octet)i;

        GUID_t entity_guid = {part_guid.guidPrefix, entity};

        if (i % 2)
        {
            pdp_->addReaderProxyData(entity_guid, part_guid,
                    [&entity_guid](ReaderProxyData* rdata, bool, const ParticipantProxyData&)
                    {
                        rdata->guid = entity_guid; return true;
                    });
        }
        else
        {
            pdp_->addWriterProxyData(entity_guid, part_guid,
                    [&entity_guid](WriterProxyData* wdata, bool, const ParticipantProxyData&)
                    {
                        wdata->guid = entity_guid; return true;
                    });
        }

        local_guids.push_back(entity_guid);
    }

    //! Generate other random participant and entities
    for (size_t i = 1; i < n_entities; i++)
    {
        GuidPrefix_t prefix;

        prefix.value[4] = std::rand() % 100;
        prefix.value[5] = std::rand() % 100;
        prefix.value[6] = std::rand() % 100;
        prefix.value[7] = std::rand() % 100;

        EntityId_t entity;
        entity.value[3] = (octet)i;

        GUID_t entity_guid = {prefix, entity};
        GUID_t other_part_guid = {prefix, ENTITYID_RTPSParticipant};
        pdp_->create_and_add_participant_proxy_data(other_part_guid);

        if (i % 2)
        {
            pdp_->addReaderProxyData(entity_guid, other_part_guid,
                    [&entity_guid](ReaderProxyData* rdata, bool, const ParticipantProxyData&)
                    {
                        rdata->guid = entity_guid; return true;
                    });
        }
        else
        {
            pdp_->addWriterProxyData(entity_guid, other_part_guid,
                    [&entity_guid](WriterProxyData* wdata, bool, const ParticipantProxyData&)
                    {
                        wdata->guid = entity_guid; return true;
                    });
        }
    }

    pdp_->get_all_local_proxies(output_guids);

    ASSERT_FALSE(output_guids.empty());

    for (auto& guid : output_guids)
    {
        auto it = std::find(local_guids.begin(), local_guids.end(), guid);
        ASSERT_TRUE(it != local_guids.end());
    }

#endif // FASTDDS_STATISTICS
}

TEST_F(PDPTests, iproxy_queryable_get_serialized_proxy)
{
#ifdef FASTDDS_STATISTICS

    GUID_t part_guid(GuidPrefix_t::unknown(), ENTITYID_RTPSParticipant);
    pdp_->create_and_add_participant_proxy_data(part_guid);

    CDRMessage_t part_proxy_serialized;
    ASSERT_TRUE(pdp_->get_serialized_proxy(part_guid, &part_proxy_serialized));

    GUID_t expected_participant_guid;
    part_proxy_serialized.pos = 0;
    ASSERT_TRUE(fastdds::dds::ParameterList::read_guid_from_cdr_msg(part_proxy_serialized,
            fastdds::dds::PID_PARTICIPANT_GUID, expected_participant_guid));
    ASSERT_EQ(part_guid, expected_participant_guid);

    EntityId_t entity;
    entity.value[3] = 4;//! valid reader EntityId
    GUID_t reader_guid = {GuidPrefix_t::unknown(), entity};
    pdp_->addReaderProxyData(reader_guid, part_guid,
            [&reader_guid](ReaderProxyData* rdata, bool, const ParticipantProxyData&)
            {
                rdata->guid = reader_guid;
                rdata->topic_name = "test";
                rdata->type_name = "foo";
                return true;
            });

    CDRMessage_t reader_proxy_serialized;
    ASSERT_TRUE(pdp_->get_serialized_proxy(reader_guid, &reader_proxy_serialized));

    reader_proxy_serialized.pos = 0;
    GUID_t expected_reader_guid;
    ASSERT_TRUE(fastdds::dds::ParameterList::read_guid_from_cdr_msg(reader_proxy_serialized,
            fastdds::dds::PID_ENDPOINT_GUID, expected_reader_guid));
    ASSERT_EQ(reader_guid, expected_reader_guid);

#endif // FASTDDS_STATISTICS
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
