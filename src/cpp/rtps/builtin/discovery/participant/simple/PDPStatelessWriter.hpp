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

/**
 * @file PDPStatelessWriter.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__PDPSTATELESSWRITER_HPP
#define FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__PDPSTATELESSWRITER_HPP

#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPStatelessWriter, specialization of StatelessWriter with specific behavior for PDP.
 */
class PDPStatelessWriter : public StatelessWriter
{

public:

    PDPStatelessWriter(
            RTPSParticipantImpl* participant,
            const GUID_t& guid,
            const WriterAttributes& attributes,
            FlowController* flow_controller,
            WriterHistory* history,
            WriterListener* listener);

    virtual ~PDPStatelessWriter() = default;

    //vvvvvvvvvvvvvvvvvvvvv [Exported API] vvvvvvvvvvvvvvvvvvvvv

    bool matched_reader_add_edp(
            const ReaderProxyData& data) final;

    bool matched_reader_remove(
            const GUID_t& reader_guid) final;

    //^^^^^^^^^^^^^^^^^^^^^^ [Exported API] ^^^^^^^^^^^^^^^^^^^^^^^

    //vvvvvvvvvvvvvvvvvvvvv [BaseWriter API] vvvvvvvvvvvvvvvvvvvvvv

    void unsent_change_added_to_history(
            CacheChange_t* change,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) final;

    //^^^^^^^^^^^^^^^^^^^^^^ [BaseWriter API] ^^^^^^^^^^^^^^^^^^^^^^^

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__PDPSTATELESSWRITER_HPP

