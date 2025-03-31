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

#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/LocalReaderPointer.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <utils/shared_memory/BoostAtExitRegistry.hpp>
#include <utils/SystemInfo.hpp>

#if HAVE_SECURITY
#include <security/OpenSSLInit.hpp>
#endif // HAVE_SECURITY

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl
{

public:

    typedef std::pair<RTPSParticipant*, RTPSParticipantImpl*> t_p_RTPSParticipant;

    /**
     * Get singleton instance.
     *
     * @return Shared pointer to RTPSDomainImpl singleton instance.
     */
    static std::shared_ptr<RTPSDomainImpl> get_instance();

    /**
     * Method to shut down all RTPSParticipants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainRTPSParticipant.
     *
     * \post After this call, all the pointers to RTPS entities are invalidated and their use may
     *       result in undefined behaviour.
     */
    static void stopAll();

    /**
     * @brief Create a RTPSParticipant.
     * @param domain_id DomainId to be used by the RTPSParticipant (80 by default).
     * @param enabled True if the RTPSParticipant should be enabled on creation. False if it will be enabled later with RTPSParticipant::enable()
     * @param attrs RTPSParticipant Attributes.
     * @param plisten Pointer to the ParticipantListener.
     * @return Pointer to the RTPSParticipant.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSParticipant() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    static RTPSParticipant* createParticipant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten);

    /**
     * @brief Create a RTPSParticipant as default server or client if ROS_MASTER_URI environment variable is set.
     * The RTPSParticipant is created as a ros easy mode client and its corresponding easy mode server is spawned
     * if at least one of the following conditions are met:
     *
     * CONDITION_A:
     *  - `easy_mode_ip` member of the input RTPSParticipantAttributes is a non-empty string.
     *
     * CONDITION_B:
     *  - ROS2_EASY_MODE_URI environment variable is set.
     *
     * In case of both conditions are met at the same time, the value of `easy_mode_ip` member is used
     * as the easy mode server IP and ROS2_EASY_MODE_URI value is ignored. A warning log is displayed in this case.
     *
     * @param domain_id DomainId to be used by the RTPSParticipant.
     * @param enabled True if the RTPSParticipant should be enabled on creation. False if it will be enabled later with RTPSParticipant::enable()
     * @param attrs RTPSParticipant Attributes.
     * @param plisten Pointer to the ParticipantListener.
     * @return Pointer to the RTPSParticipant or nullptr in the following cases:
     *
     *        - The input attributes are not overriden by the environment variables.
     *          In this case no errors ocurred,
     *          but preconditions are not met and the entire Participant setup is skipped.
     *
     *        - An error ocurred during the RTPSParticipant creation.
     *
     *        - RTPSParticipant is created correctly, but an error ocurred during the Easy Mode discovery server launch.
     *          In this case, the RTPSParticipant is removed before returning.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSParticipant() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    static RTPSParticipant* create_client_server_participant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten);

    /**
     * Remove a RTPSWriter.
     * @param writer Pointer to the writer you want to remove.
     * @return  True if correctly removed.
     */
    static bool removeRTPSWriter(
            RTPSWriter* writer);

    /**
     * Remove a RTPSReader.
     * @param reader Pointer to the reader you want to remove.
     * @return  True if correctly removed.
     */
    static bool removeRTPSReader(
            RTPSReader* reader);

    /**
     * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
     * @param [in] p Pointer to the RTPSParticipant;
     * @return True if correct.
     */
    static bool removeRTPSParticipant(
            RTPSParticipant* p);

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

    /**
     * Create a RTPSWriter in a participant.
     * @param p Pointer to the RTPSParticipant.
     * @param entity_id Specific entity id to use for the created writer.
     * @param watt Writer Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @return Pointer to the created RTPSWriter.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSWriter() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    static RTPSWriter* create_rtps_writer(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            WriterAttributes& watt,
            WriterHistory* hist,
            WriterListener* listen);

    /**
     * Creates the guid of a participant given its identifier.
     * @param [in, out] participant_id   Participant identifier for which to generate the GUID.
     *                                   When negative, it will be modified to the first non-existent participant id.
     * @param [out]     guid             GUID corresponding to participant_id
     *
     * @return True value if guid was created. False in other case.
     */
    static bool create_participant_guid(
            int32_t& participant_id,
            GUID_t& guid);

    /**
     * Find a participant given its GUID.
     *
     * @param [in] guid GUID of the participant to find
     *
     * @return The pointer to the corresponding participant implementation, nullptr if not found.
     */
    static RTPSParticipantImpl* find_local_participant(
            const GUID_t& guid);

    /**
     * Find a local-process reader.
     *
     * @param reader_guid GUID of the local reader to search.
     *
     * @returns A pointer to a local reader given its endpoint guid, or nullptr if not found.
     */
    static std::shared_ptr<LocalReaderPointer> find_local_reader(
            const GUID_t& reader_guid);

    /**
     * Find a local-process writer.
     *
     * @param writer_guid GUID of the local writer to search.
     *
     * @returns A pointer to a local writer given its endpoint guid, or nullptr if not found.
     */
    static BaseWriter* find_local_writer(
            const GUID_t& writer_guid);

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

    /**
     * Callback run when the monitored environment file is modified
     */
    static void file_watch_callback();

    /**
     * Method to set the configuration of the threads created by the file watcher for the environment file.
     * In order for these settings to take effect, this method must be called before the first call
     * to @ref createParticipant.
     *
     * @param watch_thread     Settings for the thread watching the environment file.
     * @param callback_thread  Settings for the thread executing the callback when the environment file changed.
     */
    static void set_filewatch_thread_config(
            const fastdds::rtps::ThreadSettings& watch_thread,
            const fastdds::rtps::ThreadSettings& callback_thread);

    /**
     * @brief Get the library settings.
     *
     * @param library_settings LibrarySettings reference where the settings are returned.
     * @return True.
     */
    static bool get_library_settings(
            fastdds::LibrarySettings& library_settings);

    /**
     * @brief Set the library settings.
     *
     * @param library_settings LibrarySettings to be set.
     * @return False if there is any RTPSParticipant already created.
     *         True if correctly set.
     */
    static bool set_library_settings(
            const fastdds::LibrarySettings& library_settings);

    /**
     * @brief Return the ITypeObjectRegistry member to access the interface for the public API.
     *
     * @return const xtypes::ITypeObjectRegistry reference.
     */
    static fastdds::dds::xtypes::ITypeObjectRegistry& type_object_registry();

    /**
     * @brief Return the TypeObjectRegistry member to access the  API.
     *
     * @return const xtypes::TypeObjectRegistry reference.
     */
    static fastdds::dds::xtypes::TypeObjectRegistry& type_object_registry_observer();

    /**
     * @brief Run the Easy Mode discovery server using the Fast DDS CLI command
     *
     * @param domain_id Domain ID to use for the discovery server
     * @param easy_mode_ip IP address to use for the discovery server
     *
     * @return True if the server was successfully started, false otherwise.
     */
    static bool run_easy_mode_discovery_server(
            uint32_t domain_id,
            const std::string& easy_mode_ip);

private:

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
