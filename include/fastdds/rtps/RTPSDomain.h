// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file RTPSDomain.h
 */

#ifndef _FASTDDS_RTPS_DOMAIN_H_
#define _FASTDDS_RTPS_DOMAIN_H_

#include <atomic>
#include <mutex>
#include <set>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/history/IChangePool.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class RTPSParticipant;
class RTPSParticipantListener;
class RTPSWriter;
class WriterAttributes;
class WriterHistory;
class WriterListener;
class RTPSReader;
class ReaderAttributes;
class ReaderHistory;
class ReaderListener;
class RTPSDomainImpl;

/**
 * Class RTPSDomain,it manages the creation and destruction of RTPSParticipant RTPSWriter and RTPSReader. It stores
 * a list of all created RTPSParticipant. It has only static methods.
 * @ingroup RTPS_MODULE
 */
class RTPSDomain
{
public:

    /**
     * Method to shut down all RTPSParticipants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainRTPSParticipant.
     *
     * \post After this call, all the pointers to RTPS entities are invalidated and their use may
     *       result in undefined behaviour.
     */
    RTPS_DllAPI static void stopAll();

    /**
     * @brief Create a RTPSParticipant.
     * @param domain_id DomainId to be used by the RTPSParticipant (80 by default).
     * @param attrs RTPSParticipant Attributes.
     * @param plisten Pointer to the ParticipantListener.
     * @return Pointer to the RTPSParticipant.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSParticipant() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSParticipant* createParticipant(
            uint32_t domain_id,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten = nullptr);

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
    RTPS_DllAPI static RTPSParticipant* createParticipant(
            uint32_t domain_id,
            bool enabled,
            const RTPSParticipantAttributes& attrs,
            RTPSParticipantListener* plisten = nullptr);

    /**
     * Create a RTPSWriter in a participant.
     * @param p Pointer to the RTPSParticipant.
     * @param watt Writer Attributes.
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @return Pointer to the created RTPSWriter.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSWriter() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSWriter* createRTPSWriter(
            RTPSParticipant* p,
            WriterAttributes& watt,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    /**
     * Create a RTPSWriter in a participant using a custom payload pool.
     * @param p Pointer to the RTPSParticipant.
     * @param watt Writer Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @return Pointer to the created RTPSWriter.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSWriter() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSWriter* createRTPSWriter(
            RTPSParticipant* p,
            WriterAttributes& watt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    /**
     * Create a RTPSWriter in a participant using a custom payload pool.
     * @param p Pointer to the RTPSParticipant.
     * @param watt Writer Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param change_pool Shared pointer to the IChangePool
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @return Pointer to the created RTPSWriter.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSWriter() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSWriter* createRTPSWriter(
            RTPSParticipant* p,
            WriterAttributes& watt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    /**
     * Create a RTPSWriter in a participant using a custom payload pool.
     * @param p Pointer to the RTPSParticipant.
     * @param entity_id Specific entity id to use for the created writer.
     * @param watt Writer Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param change_pool Shared pointer to the IChangePool
     * @param hist Pointer to the WriterHistory.
     * @param listen Pointer to the WriterListener.
     * @return Pointer to the created RTPSWriter.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSWriter() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSWriter* createRTPSWriter(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            WriterAttributes& watt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

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
    RTPS_DllAPI static RTPSWriter* createRTPSWriter(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            WriterAttributes& watt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    /**
     * Remove a RTPSWriter.
     * @param writer Pointer to the writer you want to remove.
     * @return  True if correctly removed.
     */
    RTPS_DllAPI static bool removeRTPSWriter(
            RTPSWriter* writer);

    /**
     * Create a RTPSReader in a participant.
     * @param p Pointer to the RTPSParticipant.
     * @param ratt Reader Attributes.
     * @param hist Pointer to the ReaderHistory.
     * @param listen Pointer to the ReaderListener.
     * @return Pointer to the created RTPSReader.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSReader() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSReader* createRTPSReader(
            RTPSParticipant* p,
            ReaderAttributes& ratt,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    /**
     * Create a RTPReader in a participant using a custom payload pool.
     * @param p Pointer to the RTPSParticipant.
     * @param ratt Reader Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param hist Pointer to the ReaderHistory.
     * @param listen Pointer to the ReaderListener.
     * @return Pointer to the created RTPSReader.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSReader() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSReader* createRTPSReader(
            RTPSParticipant* p,
            ReaderAttributes& ratt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    /**
     * Create a RTPSReader in a participant using a custom payload pool.
     * @param p Pointer to the RTPSParticipant.
     * @param entity_id Specific entity id to use for the created reader.
     * @param ratt Reader Attributes.
     * @param payload_pool Shared pointer to the IPayloadPool
     * @param hist Pointer to the ReaderHistory.
     * @param listen Pointer to the ReaderListener.
     * @return Pointer to the created RTPSReader.
     *
     * \warning The returned pointer is invalidated after a call to removeRTPSReader() or stopAll(),
     *          so its use may result in undefined behaviour.
     */
    RTPS_DllAPI static RTPSReader* createRTPSReader(
            RTPSParticipant* p,
            const EntityId_t& entity_id,
            ReaderAttributes& ratt,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen = nullptr);

    /**
     * Remove a RTPSReader.
     * @param reader Pointer to the reader you want to remove.
     * @return  True if correctly removed.
     */
    RTPS_DllAPI static bool removeRTPSReader(
            RTPSReader* reader);

    /**
     * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
     * @param[in] p Pointer to the RTPSParticipant;
     * @return True if correct.
     */
    RTPS_DllAPI static bool removeRTPSParticipant(
            RTPSParticipant* p);

private:

    RTPSDomain() = delete;

    /**
     * DomainRTPSParticipant destructor
     */
    ~RTPSDomain() = delete;
};

} // namespace rtps
} /* namespace fastrtps  */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DOMAIN_H_*/
