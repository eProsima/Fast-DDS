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

#ifndef FASTDDS_TEST_MOCK_FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTMOCK_HPP
#define FASTDDS_TEST_MOCK_FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTMOCK_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <mock/MockCompanion.hpp>
#include <statistics/rtps/monitor-service/Interfaces.hpp>
#include <statistics/types/monitorservice_types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Expectations of a RTPSParticipant created with the mock implementation, reached with mock::mock_of().
 *
 * By default getGuid(), get_attributes() and get_const_attributes() return the data members below, and
 * copy_attributes() and get_mutable_attributes() are built from @c attributes. The statistics listener and discovery
 * data methods succeed. get_resource_event() returns a pointer here, because the real method returns a reference: the
 * test has to set the ResourceEvent to return before the participant is asked for it.
 */
class RTPSParticipantMock
{
public:

    RTPSParticipantMock()
    {
        using testing::_;
        using testing::An;
        using testing::Return;
        using testing::ReturnRef;

        ON_CALL(*this, getGuid()).WillByDefault(ReturnRef(guid));
        ON_CALL(*this, get_attributes()).WillByDefault(ReturnRef(attributes));
        ON_CALL(*this, get_const_attributes()).WillByDefault(ReturnRef(const_attributes));
        ON_CALL(*this, get_mutable_attributes()).WillByDefault([this]()
                {
                    return RTPSParticipantMutableAttributes(attributes);
                });
        ON_CALL(*this, copy_attributes()).WillByDefault([this]()
                {
                    return attributes;
                });
        ON_CALL(*this, add_statistics_listener(_, _)).WillByDefault(Return(true));
        ON_CALL(*this, remove_statistics_listener(_, _)).WillByDefault(Return(true));
        ON_CALL(*this, fill_discovery_data_from_cdr_message(An<ParticipantBuiltinTopicData&>(), _))
                .WillByDefault(Return(true));
        ON_CALL(*this, fill_discovery_data_from_cdr_message(An<PublicationBuiltinTopicData&>(), _))
                .WillByDefault(Return(true));
        ON_CALL(*this, fill_discovery_data_from_cdr_message(An<SubscriptionBuiltinTopicData&>(), _))
                .WillByDefault(Return(true));
    }

    MOCK_METHOD(const GUID_t&, getGuid, (), (const));
    MOCK_METHOD(void, announceRTPSParticipantState, ());
    MOCK_METHOD(void, stopRTPSParticipantAnnouncement, ());
    MOCK_METHOD(void, resetRTPSParticipantAnnouncement, ());
    MOCK_METHOD(bool, newRemoteWriterDiscovered, (const GUID_t&, int16_t));
    MOCK_METHOD(bool, newRemoteReaderDiscovered, (const GUID_t&, int16_t));
    MOCK_METHOD(uint32_t, getRTPSParticipantID, (), (const));
    MOCK_METHOD(bool, register_writer, (RTPSWriter*, const TopicDescription&, const fastdds::dds::WriterQos&));
    MOCK_METHOD(dds::ReturnCode_t, register_writer,
            (RTPSWriter*, const TopicDescription&, const PublicationBuiltinTopicData&));
    MOCK_METHOD(bool, register_reader,
            (RTPSReader*, const TopicDescription&, const fastdds::dds::ReaderQos&, const ContentFilterProperty*));
    MOCK_METHOD(dds::ReturnCode_t, register_reader,
            (RTPSReader*, const TopicDescription&, const SubscriptionBuiltinTopicData&,
            const ContentFilterProperty*));
    MOCK_METHOD(void, update_attributes, (const RTPSParticipantAttributes&));
    MOCK_METHOD(bool, update_writer, (RTPSWriter*, const fastdds::dds::WriterQos&));
    MOCK_METHOD(bool, update_reader, (RTPSReader*, const fastdds::dds::ReaderQos&, const ContentFilterProperty*));
    MOCK_METHOD(std::vector<std::string>, getParticipantNames, (), (const));
    MOCK_METHOD(const RTPSParticipantAttributes&, get_attributes, (), (const));
    MOCK_METHOD(const RTPSParticipantConstantAttributes&, get_const_attributes, (), (const));
    MOCK_METHOD(const RTPSParticipantMutableAttributes, get_mutable_attributes, (), (const));
    MOCK_METHOD(RTPSParticipantAttributes, copy_attributes, (), (const));
    MOCK_METHOD(uint32_t, getMaxMessageSize, (), (const));
    MOCK_METHOD(uint32_t, getMaxDataSize, (), (const));
    MOCK_METHOD(ResourceEvent*, get_resource_event, (), (const));
    MOCK_METHOD(WLP*, wlp, (), (const));
    MOCK_METHOD(bool, get_new_entity_id, (EntityId_t &));
    MOCK_METHOD(void, set_check_type_function, (std::function<bool(const std::string&)>&&));
    MOCK_METHOD(void, set_listener, (RTPSParticipantListener*));
    MOCK_METHOD(uint32_t, get_domain_id, (), (const));
    MOCK_METHOD(void, enable, ());
    MOCK_METHOD(bool, ignore_participant, (const GuidPrefix_t&));
    MOCK_METHOD(bool, ignore_writer, (const GUID_t&));
    MOCK_METHOD(bool, ignore_reader, (const GUID_t&));
    MOCK_METHOD(std::vector<TransportNetmaskFilterInfo>, get_netmask_filter_info, (), (const));
    MOCK_METHOD(bool, get_publication_info, (PublicationBuiltinTopicData&, const GUID_t&), (const));
    MOCK_METHOD(bool, get_subscription_info, (SubscriptionBuiltinTopicData&, const GUID_t&), (const));
#if HAVE_SECURITY
    MOCK_METHOD(bool, is_security_enabled_for_writer, (const WriterAttributes&));
    MOCK_METHOD(bool, is_security_enabled_for_reader, (const ReaderAttributes&));
#endif // if HAVE_SECURITY
    MOCK_METHOD(bool, add_statistics_listener, (std::shared_ptr<fastdds::statistics::IListener>, uint32_t));
    MOCK_METHOD(bool, remove_statistics_listener, (std::shared_ptr<fastdds::statistics::IListener>, uint32_t));
    MOCK_METHOD(void, set_enabled_statistics_writers_mask, (uint32_t));
    MOCK_METHOD(const fastdds::statistics::rtps::IStatusObserver*, create_monitor_service,
            (fastdds::statistics::rtps::IStatusQueryable&));
    MOCK_METHOD(bool, create_monitor_service, ());
    MOCK_METHOD(bool, is_monitor_service_created, (), (const));
    MOCK_METHOD(bool, enable_monitor_service, (), (const));
    MOCK_METHOD(bool, disable_monitor_service, (), (const));
    MOCK_METHOD(bool, fill_discovery_data_from_cdr_message,
            (ParticipantBuiltinTopicData&, const fastdds::statistics::MonitorServiceStatusData&));
    MOCK_METHOD(bool, fill_discovery_data_from_cdr_message,
            (PublicationBuiltinTopicData&, const fastdds::statistics::MonitorServiceStatusData&));
    MOCK_METHOD(bool, fill_discovery_data_from_cdr_message,
            (SubscriptionBuiltinTopicData&, const fastdds::statistics::MonitorServiceStatusData&));

    //! GUID returned by default by getGuid().
    GUID_t guid;
    //! Attributes returned by default by get_attributes(), copy_attributes() and get_mutable_attributes().
    RTPSParticipantAttributes attributes;
    //! Attributes returned by default by get_const_attributes().
    RTPSParticipantConstantAttributes const_attributes;

};

} // namespace rtps

namespace mock {

template<>
struct MockOf<rtps::RTPSParticipant>
{
    using type = testing::NiceMock<rtps::RTPSParticipantMock>;
};

} // namespace mock
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_TEST_MOCK_FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTMOCK_HPP
