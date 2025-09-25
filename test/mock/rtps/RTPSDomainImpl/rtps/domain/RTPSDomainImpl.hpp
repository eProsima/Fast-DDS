// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RTPS__RTPSDOMAINIMPL_HPP
#define FASTDDS_RTPS__RTPSDOMAINIMPL_HPP

#include <memory>

#include <gmock/gmock.h>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>

#include <rtps/domain/IDomainImpl.hpp>
#include <rtps/reader/LocalReaderPointer.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseWriter;
class IChangePool;
class RTPSReader;
class RTPSWriter;

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl : public testing::NiceMock<IDomainImpl>
{
public:

    ~RTPSDomainImpl() override = default;

    static std::shared_ptr<IDomainImpl> get_instance()
    {
        static std::shared_ptr<RTPSDomainImpl> instance = std::make_shared<RTPSDomainImpl>();
        return instance;
    }

    MOCK_METHOD(void, stop_all, (), (override));
    MOCK_METHOD(RTPSParticipant*, create_participant,
            (uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*),
            (override));
    MOCK_METHOD(RTPSParticipant*, create_client_server_participant,
            (uint32_t, bool, const RTPSParticipantAttributes&, RTPSParticipantListener*),
            (override));
    MOCK_METHOD(bool, remove_writer,
            (RTPSWriter*), (override));
    MOCK_METHOD(bool, remove_reader,
            (RTPSReader*), (override));
    MOCK_METHOD(bool, remove_participant,
            (RTPSParticipant*), (override));
    MOCK_METHOD(RTPSWriter*, create_writer,
            (RTPSParticipant*, const EntityId_t&, WriterAttributes&, WriterHistory*, WriterListener*),
            (override));
    MOCK_METHOD(RTPSParticipantImpl*, find_participant,
            (const GUID_t&), (override));
    MOCK_METHOD(void, find_reader,
            (std::shared_ptr<LocalReaderPointer>&, const GUID_t&), (override));
    MOCK_METHOD(BaseWriter*, find_writer,
            (const GUID_t&), (override));
    MOCK_METHOD(void, file_watch_callback, (), (override));
    MOCK_METHOD(void, set_filewatch_thread_config,
            (const ThreadSettings&, const ThreadSettings&), (override));
    MOCK_METHOD(bool, run_easy_mode_discovery_server,
            (uint32_t, const std::string&), (override));

    /**
     * Check whether intraprocess delivery should be used between two GUIDs.
     *
     * @param local_guid    GUID of the local endpoint performing the query.
     * @param matched_guid  GUID being queried about.
     *
     * @returns true when intraprocess delivery should be used, false otherwise.
     */
    static bool should_intraprocess_between(
            const GUID_t& /* local_guid */,
            const GUID_t& /* matched_guid */)
    {
        return false;
    }

    bool create_participant_guid(
            int32_t& /*participant_id*/,
            GUID_t& guid) override
    {
        guid.guidPrefix.value[11] = 1;
        return true;
    }

    bool get_library_settings(
            fastdds::LibrarySettings&) override
    {
        return true;
    }

    bool set_library_settings(
            const fastdds::LibrarySettings&) override
    {
        return true;
    }

    fastdds::dds::xtypes::ITypeObjectRegistry& type_object_registry() override
    {
        return type_object_registry_;
    }

    fastdds::dds::xtypes::TypeObjectRegistry& type_object_registry_observer() override
    {
        return type_object_registry_;
    }

    eprosima::fastdds::dds::xtypes::TypeObjectRegistry type_object_registry_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS__RTPSDOMAINIMPL_HPP
