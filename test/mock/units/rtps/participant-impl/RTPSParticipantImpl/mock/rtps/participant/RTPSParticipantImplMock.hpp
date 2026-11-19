// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_TEST_MOCK_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPLMOCK_HPP
#define FASTDDS_TEST_MOCK_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPLMOCK_HPP

#include <cstdint>
#include <map>

#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>
#include <fastdds/utils/TypePropagation.hpp>
#include <mock/MockCompanion.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Participant listener on which tests set expectations.
 */
class MockParticipantListener : public RTPSParticipantListener
{
public:

    void on_participant_discovery(
            RTPSParticipant* participant,
            ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override
    {
        on_participant_discovery_mock(participant, status, info, should_be_ignored);
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD4(on_participant_discovery_mock,
            void (RTPSParticipant*, ParticipantDiscoveryStatus, const ParticipantBuiltinTopicData&, bool&));
    // *INDENT-ON*

#if HAVE_SECURITY
    void onParticipantAuthentication(
            RTPSParticipant* participant,
            ParticipantAuthenticationInfo&& info) override
    {
        onParticipantAuthentication(participant, info);
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD2(onParticipantAuthentication, void (RTPSParticipant*, const ParticipantAuthenticationInfo&));
    // *INDENT-ON*
#endif // if HAVE_SECURITY
};

/**
 * Expectations of an RTPSParticipantImpl created with the mock implementation, reached with mock::mock_of().
 * The participant has no transports, builtin protocols nor endpoints of its own: endpoints are created by the tests in
 * the actions of createWriter() and createReader(). The participant keeps the endpoints created, and deletes them in
 * deleteUserEndpoint().
 */
class RTPSParticipantImplMock
{
public:

    //! Endpoints created through the participant, by GUID.
    std::map<GUID_t, Endpoint*> endpoints;

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD0(getMaxDataSize, uint32_t());

    MOCK_METHOD0(get_domain_id, uint32_t());

    MOCK_METHOD1(setGuid, void(GUID_t& guid));

    MOCK_METHOD0(pdp, PDP* ());

    MOCK_METHOD0(pdpsimple, PDPSimple* ());

    MOCK_METHOD0(wlp, WLP* ());

    MOCK_METHOD0(typelookup_manager, fastdds::dds::builtin::TypeLookupManager* ());

    MOCK_METHOD6(createWriter, bool(
            RTPSWriter** writer,
            WriterAttributes& param,
            WriterHistory* hist,
            WriterListener* listen,
            const EntityId_t& entity_id,
            bool is_builtin));

    MOCK_METHOD7(createReader, bool(
            RTPSReader** reader,
            ReaderAttributes& param,
            ReaderHistory* hist,
            ReaderListener* listen,
            const EntityId_t& entity_id,
            bool is_builtin,
            bool enable));

    MOCK_METHOD3(register_writer, dds::ReturnCode_t(
            RTPSWriter* writer,
            const TopicDescription& topic,
            const PublicationBuiltinTopicData& pub_builtin_topic_data));

    MOCK_METHOD4(register_reader, dds::ReturnCode_t(
            RTPSReader* reader,
            const TopicDescription& topic,
            const SubscriptionBuiltinTopicData& sub_builtin_topic_data,
            const ContentFilterProperty* content_filter));

    MOCK_METHOD0(should_send_optional_qos, bool());

    MOCK_METHOD2(pairing_remote_reader_with_local_writer_after_security, bool(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data));

    MOCK_METHOD2(pairing_remote_writer_with_local_reader_after_security, bool(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data));

    MOCK_METHOD0(should_match_local_endpoints, bool());

    MOCK_METHOD1(ignore_participant, bool(const GuidPrefix_t& participant_guid));

    MOCK_METHOD1(update_removed_participant, void(const LocatorList_t& remote_participant_locators));

    MOCK_METHOD0(type_propagation, dds::utils::TypePropagation());

    MOCK_METHOD3(notify_reader_discovery, void(
            ReaderDiscoveryStatus reason,
            const SubscriptionBuiltinTopicData& info,
            RTPSParticipantListener* listener));

    MOCK_METHOD3(notify_writer_discovery, void(
            WriterDiscoveryStatus reason,
            const PublicationBuiltinTopicData& info,
            RTPSParticipantListener* listener));
    // *INDENT-ON*
};

namespace detail {

//! Listener of a TestRTPSParticipantImpl, constructed before the participant.
struct TestRTPSParticipantImplListener
{
    testing::NiceMock<MockParticipantListener> listener;
};

} // namespace detail

/**
 * RTPSParticipantImpl for tests needing a participant, with a MockParticipantListener as listener.
 */
class TestRTPSParticipantImpl : private detail::TestRTPSParticipantImplListener, public RTPSParticipantImpl
{
public:

    explicit TestRTPSParticipantImpl(
            const GuidPrefix_t& guid_prefix = GuidPrefix_t(),
            const RTPSParticipantAttributes& attributes = RTPSParticipantAttributes())
        : RTPSParticipantImpl(0, attributes, guid_prefix, nullptr, &listener)
    {
    }

    /**
     * Get the listener of the participant.
     * @return Listener on which tests set expectations.
     */
    testing::NiceMock<MockParticipantListener>& mock_listener()
    {
        return listener;
    }

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::RTPSParticipantImpl>
{
    using type = testing::NiceMock<rtps::RTPSParticipantImplMock>;
};

/**
 * Get the companion of a TestRTPSParticipantImpl, which is the one of its RTPSParticipantImpl base.
 * @param participant Instance created with the mock implementation.
 * @return Companion holding the expectations of the instance.
 */
inline MockOf<rtps::RTPSParticipantImpl>::type& mock_of(
        const rtps::TestRTPSParticipantImpl* participant)
{
    return MockCompanion<rtps::RTPSParticipantImpl>::get(participant);
}

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_RTPS_PARTICIPANT__RTPSPARTICIPANTIMPLMOCK_HPP
