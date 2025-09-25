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

#include <chrono>
#include <memory>
#include <unordered_map>

#if defined(_WIN32) || defined(__unix__)
#include <FileWatch.hpp>
#endif // defined(_WIN32) || defined(__unix__)

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/domain/IDomainImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/LocalReaderPointer.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <utils/shared_memory/BoostAtExitRegistry.hpp>
#include <utils/SystemInfo.hpp>

#if HAVE_SECURITY
#include <security/OpenSSLInit.hpp>
#endif // HAVE_SECURITY

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <rtps/builtin/discovery/participant/PDP.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl : public IDomainImpl
{

public:

    ~RTPSDomainImpl() override = default;

    typedef std::pair<RTPSParticipant*, RTPSParticipantImpl*> t_p_RTPSParticipant;

    /**
     * Get singleton instance.
     *
     * @return Shared pointer to RTPSDomainImpl singleton instance.
     */
    static std::shared_ptr<IDomainImpl> get_instance();

    void stop_all() override;

    RTPSParticipant* create_participant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten) override;

    RTPSParticipant* create_client_server_participant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten) override;

    bool remove_writer(
            RTPSWriter* writer) override;

    bool remove_reader(
            RTPSReader* reader) override;

    bool remove_participant(
            RTPSParticipant* p) override;

    /**
     * Fills RTPSParticipantAttributes to create a RTPSParticipant as default server or client
     * if ROS_MASTER_URI environment variable is set.
     * It also configures ROS 2 Easy Mode IP if ROS2_EASY_MODE_URI environment variable is set
     * and it was empty in the input attributes.
     *
     * @param domain_id DDS domain associated
     * @param [in, out] attrs RTPSParticipant Attributes.
     * @return True if the attributes were successfully modified,
     * false if an error occurred or environment variable not set
     *
     */
    static bool client_server_environment_attributes_override(
            uint32_t domain_id,
            RTPSParticipantAttributes& attrs);

    RTPSWriter* create_writer(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            WriterAttributes& watt,
            WriterHistory* hist,
            WriterListener* listen) override;

    bool create_participant_guid(
            int32_t& participant_id,
            GUID_t& guid) override;

    RTPSParticipantImpl* find_participant(
            const GUID_t& guid) override;

    void find_reader(
            std::shared_ptr<LocalReaderPointer>& local_reader,
            const GUID_t& reader_guid) override;

    BaseWriter* find_writer(
            const GUID_t& writer_guid) override;

    /**
     * Check whether intraprocess delivery should be used between two GUIDs.
     *
     * @param local_guid    GUID of the local endpoint performing the query.
     * @param matched_guid  GUID being queried about.
     *
     * @returns true when intraprocess delivery should be used, false otherwise.
     */
    static bool should_intraprocess_between(
            const GUID_t& local_guid,
            const GUID_t& matched_guid);

    void file_watch_callback() override;

    void set_filewatch_thread_config(
            const fastdds::rtps::ThreadSettings& watch_thread,
            const fastdds::rtps::ThreadSettings& callback_thread) override;

    bool get_library_settings(
            fastdds::LibrarySettings& library_settings) override;

    bool set_library_settings(
            const fastdds::LibrarySettings& library_settings) override;

    fastdds::dds::xtypes::ITypeObjectRegistry& type_object_registry() override;

    fastdds::dds::xtypes::TypeObjectRegistry& type_object_registry_observer() override;

    bool run_easy_mode_discovery_server(
            uint32_t domain_id,
            const std::string& easy_mode_ip) override;

protected:

    /**
     * @brief Get Id to create a RTPSParticipant.
     *
     * This function assumes m_mutex is already locked by the caller.
     *
     * @return Different ID for each call.
     */
    uint32_t getNewId();

    bool prepare_participant_id(
            int32_t input_id,
            uint32_t& participant_id);

    /**
     * Reserves a participant id.
     * @param [in, out] participant_id   Participant identifier for reservation.
     *                                   When negative, it will be modified to the first non-reserved participant id.
     *
     * @return True value if reservation was possible. False in other case.
     */
    bool reserve_participant_id(
            int32_t& participant_id);

    uint32_t get_id_for_prefix(
            uint32_t participant_id);

    void removeRTPSParticipant_nts(
            t_p_RTPSParticipant&);

    std::shared_ptr<eprosima::detail::BoostAtExitRegistry> boost_singleton_handler_ { eprosima::detail::
                                                                                              BoostAtExitRegistry::
                                                                                              get_instance() };
#if HAVE_SECURITY
    std::shared_ptr<security::OpenSSLInit> openssl_singleton_handler_{ security::OpenSSLInit::get_instance() };
#endif // HAVE_SECURITY

    std::mutex m_mutex;

    std::vector<t_p_RTPSParticipant> m_RTPSParticipants;

    struct ParticipantIDState
    {
        uint32_t counter = 0;
        bool reserved = false;
        bool used = false;
    };

    std::unordered_map<uint32_t, ParticipantIDState> m_RTPSParticipantIDs;

    FileWatchHandle file_watch_handle_;
    fastdds::rtps::ThreadSettings watch_thread_config_;
    fastdds::rtps::ThreadSettings callback_thread_config_;

    eprosima::fastdds::dds::xtypes::TypeObjectRegistry type_object_registry_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_RTPS__RTPSDOMAINIMPL_HPP
