// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file StatefulPersistentWriter.hpp
 */
#ifndef RTPS_WRITER__STATEFULPERSISTENTWRITER_HPP
#define RTPS_WRITER__STATEFULPERSISTENTWRITER_HPP

#include <rtps/writer/PersistentWriter.hpp>
#include <rtps/writer/StatefulWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class IPersistenceService;

/**
 * Class StatefulPersistentWriter, specialization of StatefulWriter that manages history persistence.
 * @ingroup WRITER_MODULE
 */
class StatefulPersistentWriter : public StatefulWriter, private PersistentWriter
{
    void print_inconsistent_acknack(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& min_requested_sequence_number,
            const SequenceNumber_t& max_requested_sequence_number,
            const SequenceNumber_t& next_sequence_number) final;

    bool log_error_printed_ = false;

public:

    StatefulPersistentWriter(
            RTPSParticipantImpl*,
            const GUID_t& guid,
            const WriterAttributes& att,
            fastdds::rtps::FlowController* flow_controller,
            WriterHistory* hist,
            WriterListener* listen = nullptr,
            IPersistenceService* persistence = nullptr);

    virtual ~StatefulPersistentWriter();

    /**
     * Add a specific change to all ReaderLocators.
     * @param p Pointer to the change.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     */
    void unsent_change_added_to_history(
            CacheChange_t* p,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) final;

    /**
     * Indicate the writer that a change has been removed by the history due to some HistoryQos requirement.
     * @param a_change Pointer to the change that is going to be removed.
     * @param [in] max_blocking_time Maximum time this method has to complete the task.
     * @return True if removed correctly.
     */
    bool change_removed_by_history(
            CacheChange_t* a_change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) final;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__STATEFULPERSISTENTWRITER_HPP
