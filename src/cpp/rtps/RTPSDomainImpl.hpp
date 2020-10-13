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

#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <chrono>
#include <thread>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @brief Class RTPSDomainImpl, contains the private implementation of the RTPSDomain
 * @ingroup RTPS_MODULE
 */
class RTPSDomainImpl
{
public:

    /**
     * Creates the guid of a participant given its identifier.
     * @param [in, out] participant_id   Participant identifier for which to generate the GUID.
     *                                   When negative, it will be modified to the first non-existent participant id.
     * @param [out]     guid             GUID corresponding to participant_id
     */
    static void create_participant_guid(
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
     * Apply a predicate to every local participant.
     *
     * Will apply the predicate to all the participants registered by a call to RTPSDomain::createParticipant.
     *
     * @param pred   Unary function that accepts a std::pair<RTPSParticipant*,RTPSParticipantImpl*> const ref as
     *               argument and returns a value convertible to bool.
     *               The value returned indicates whether the loop should continue or not.
     *               The function shall not modify its argument.
     *               This can either be a function pointer or a function object.
     */
    template<class UnaryPredicate>
    static void for_each_participant(
            UnaryPredicate pred)
    {
        std::lock_guard<std::mutex> guard(RTPSDomain::m_mutex);
        for (const RTPSDomain::t_p_RTPSParticipant& participant : RTPSDomain::m_RTPSParticipants)
        {
            if (!pred(participant))
            {
                break;
            }
        }
    }

    /**
     * Find a local-process reader.
     *
     * @param reader_guid GUID of the local reader to search.
     *
     * @returns A pointer to a local reader given its endpoint guid, or nullptr if not found.
     */
    static RTPSReader* find_local_reader(
            const GUID_t& reader_guid);

    /**
     * Find a local-process writer.
     *
     * @param writer_guid GUID of the local writer to search.
     *
     * @returns A pointer to a local writer given its endpoint guid, or nullptr if not found.
     */
    static RTPSWriter* find_local_writer(
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
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
