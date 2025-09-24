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


#ifndef SRC_CPP_RTPS_DOMAIN__IDOMAINIMPL_HPP
#define SRC_CPP_RTPS_DOMAIN__IDOMAINIMPL_HPP

#include <cstdint>
#include <memory>
#include <string>

#include <rtps/reader/LocalReaderPointer.hpp>

namespace eprosima {
namespace fastdds {

// Forward declarations
class LibrarySettings;

namespace dds {
namespace xtypes {

class ITypeObjectRegistry;
class TypeObjectRegistry;

} // namespace xtypes
} // namespace dds

namespace rtps {

// Forward declarations
class RTPSParticipant;
class RTPSParticipantImpl;
class RTPSParticipantListener;
class RTPSWriter;
class RTPSReader;
class WriterHistory;
class WriterListener;
class BaseWriter;
class RTPSParticipantAttributes;
class WriterAttributes;
struct EntityId_t;
struct GUID_t;
struct ThreadSettings;

/**
 * @brief Interface IDomainImpl, defines the required operations of the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class IDomainImpl
{

public:

    virtual ~IDomainImpl() = default;

    /**
     * Method to shut down all RTPSParticipants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainRTPSParticipant.
     *
     * \post After this call, all the pointers to RTPS entities are invalidated and their use may
     *       result in undefined behaviour.
     */
    virtual void stop_all() = 0;

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
    virtual RTPSParticipant* create_participant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten) = 0;

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
    virtual RTPSParticipant* create_client_server_participant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten) = 0;

    /**
     * Remove a RTPSWriter.
     * @param writer Pointer to the writer you want to remove.
     * @return  True if correctly removed.
     */
    virtual bool remove_writer(
            RTPSWriter* writer) = 0;

    /**
     * Remove a RTPSReader.
     * @param reader Pointer to the reader you want to remove.
     * @return  True if correctly removed.
     */
    virtual bool remove_reader(
            RTPSReader* reader) = 0;

    /**
     * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
     * @param [in] p Pointer to the RTPSParticipant;
     * @return True if correct.
     */
    virtual bool remove_participant(
            RTPSParticipant* p) = 0;

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
    virtual RTPSWriter* create_writer(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            WriterAttributes& watt,
            WriterHistory* hist,
            WriterListener* listen) = 0;

    /**
     * Creates the guid of a participant given its identifier.
     * @param [in, out] participant_id   Participant identifier for which to generate the GUID.
     *                                   When negative, it will be modified to the first non-existent participant id.
     * @param [out]     guid             GUID corresponding to participant_id
     *
     * @return True value if guid was created. False in other case.
     */
    virtual bool create_participant_guid(
            int32_t& participant_id,
            GUID_t& guid) = 0;

    /**
     * Find a participant given its GUID.
     *
     * @param [in] guid GUID of the participant to find
     *
     * @return The pointer to the corresponding participant implementation, nullptr if not found.
     */
    virtual RTPSParticipantImpl* find_participant(
            const GUID_t& guid) = 0;

    /**
     * Find a local-process reader.
     *
     * @param [in, out] local_reader Reference to the shared pointer to be set.
     * @param           reader_guid GUID of the local reader to search.
     *
     * @post If @c local_reader had a non-null value upon entry, it will not be modified.
     *       Otherwise, it will be set to point to a local reader whose GUID is the one given in @c reader_guid, or nullptr if not found.
     */
    virtual void find_reader(
            std::shared_ptr<LocalReaderPointer>& local_reader,
            const GUID_t& reader_guid) = 0;

    /**
     * Find a local-process writer.
     *
     * @param writer_guid GUID of the local writer to search.
     *
     * @returns A pointer to a local writer given its endpoint guid, or nullptr if not found.
     */
    virtual BaseWriter* find_writer(
            const GUID_t& writer_guid) = 0;

    /**
     * Callback run when the monitored environment file is modified
     */
    virtual void file_watch_callback() = 0;

    /**
     * Method to set the configuration of the threads created by the file watcher for the environment file.
     * In order for these settings to take effect, this method must be called before the first call
     * to @ref createParticipant.
     *
     * @param watch_thread     Settings for the thread watching the environment file.
     * @param callback_thread  Settings for the thread executing the callback when the environment file changed.
     */
    virtual void set_filewatch_thread_config(
            const fastdds::rtps::ThreadSettings& watch_thread,
            const fastdds::rtps::ThreadSettings& callback_thread) = 0;

    /**
     * @brief Get the library settings.
     *
     * @param library_settings LibrarySettings reference where the settings are returned.
     * @return True.
     */
    virtual bool get_library_settings(
            fastdds::LibrarySettings& library_settings) = 0;

    /**
     * @brief Set the library settings.
     *
     * @param library_settings LibrarySettings to be set.
     * @return False if there is any RTPSParticipant already created.
     *         True if correctly set.
     */
    virtual bool set_library_settings(
            const fastdds::LibrarySettings& library_settings) = 0;

    /**
     * @brief Return the ITypeObjectRegistry member to access the interface for the public API.
     *
     * @return const xtypes::ITypeObjectRegistry reference.
     */
    virtual fastdds::dds::xtypes::ITypeObjectRegistry& type_object_registry() = 0;

    /**
     * @brief Return the TypeObjectRegistry member to access the  API.
     *
     * @return const xtypes::TypeObjectRegistry reference.
     */
    virtual fastdds::dds::xtypes::TypeObjectRegistry& type_object_registry_observer() = 0;

    /**
     * @brief Run the Easy Mode discovery server using the Fast DDS CLI command
     *
     * @param domain_id Domain ID to use for the discovery server
     * @param easy_mode_ip IP address to use for the discovery server
     *
     * @return True if the server was successfully started, false otherwise.
     */
    virtual bool run_easy_mode_discovery_server(
            uint32_t domain_id,
            const std::string& easy_mode_ip) = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // SRC_CPP_RTPS_DOMAIN__IDOMAINIMPL_HPP
