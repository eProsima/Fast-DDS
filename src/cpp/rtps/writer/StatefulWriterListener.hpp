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
 * @file StatefulWriterListener.hpp
 */

#ifndef FASTDDS_RTPS_WRITER__STATEFULWRITERLISTENER_HPP
#define FASTDDS_RTPS_WRITER__STATEFULWRITERLISTENER_HPP

#include <chrono>
#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct GUID_t;
struct LocatorSelectorEntry;
struct SequenceNumber_t;

/**
 * Interface class for StatefulWriter listeners.
 */
class StatefulWriterListener
{
protected:

    StatefulWriterListener() = default;

public:

    /**
     * @brief Method called when a writer resends data to a reader.
     *
     * @param writer_guid      GUID of the writer resending the data.
     * @param reader_guid      GUID of the reader receiving the resent data.
     * @param sequence_number  Sequence number of the data being resent.
     * @param resent_bytes     Number of bytes being resent.
     * @param locators         LocatorSelectorEntry containing the locators where the reader can be reached.
     */
    virtual void on_writer_resend_data(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& sequence_number,
            uint32_t resent_bytes,
            const LocatorSelectorEntry& locators) = 0;

    /**
     * @brief Method called when a writer receives an acknowledgment for data sent to a reader.
     *
     * @param writer_guid      GUID of the writer whose data has been acknowledged.
     * @param reader_guid      GUID of the reader that acknowledged the data.
     * @param sequence_number  Sequence number of the data that has been acknowledged.
     * @param payload_length   Length of the payload that has been acknowledged.
     * @param ack_duration     Duration taken by the reader to acknowledge the data.
     * @param locators         LocatorSelectorEntry containing the locators where the reader can be reached.
     */
    virtual void on_writer_data_acknowledged(
            const GUID_t& writer_guid,
            const GUID_t& reader_guid,
            const SequenceNumber_t& sequence_number,
            uint32_t payload_length,
            const std::chrono::steady_clock::duration& ack_duration,
            const LocatorSelectorEntry& locators) = 0;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_RTPS_WRITER__STATEFULWRITERLISTENER_HPP
