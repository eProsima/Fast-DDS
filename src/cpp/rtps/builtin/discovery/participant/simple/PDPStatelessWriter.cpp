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
 * @file PDPStatelessWriter.cpp
 */

#include <rtps/builtin/discovery/participant/simple/PDPStatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPStatelessWriter::PDPStatelessWriter(
        RTPSParticipantImpl* participant,
        const GUID_t& guid,
        const WriterAttributes& attributes,
        FlowController* flow_controller,
        WriterHistory* history,
        WriterListener* listener)
    : StatelessWriter(participant, guid, attributes, flow_controller, history, listener)
{
}

bool PDPStatelessWriter::matched_reader_add_edp(
        const ReaderProxyData& data)
{
    bool ret = StatelessWriter::matched_reader_add_edp(data);
    if (ret)
    {
        // Mark new reader as interested
    }
    return ret;
}

bool PDPStatelessWriter::matched_reader_remove(
        const GUID_t& reader_guid)
{
    bool ret = StatelessWriter::matched_reader_remove(reader_guid);
    if (ret)
    {
        // Mark reader as not interested
    }
    return ret;
}

void PDPStatelessWriter::unsent_change_added_to_history(
        CacheChange_t* change,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    StatelessWriter::unsent_change_added_to_history(change, max_blocking_time);
    // mark_all_readers_as_interested();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
