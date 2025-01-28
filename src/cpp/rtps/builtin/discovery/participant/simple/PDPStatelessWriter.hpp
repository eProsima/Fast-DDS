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

#include <chrono>

#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class PDPStatelessWriter, specialization of StatelessWriter with specific behavior for PDP.
 */
class PDPStatelessWriter : public StatelessWriter, private IReaderDataFilter
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

    /**
     * @brief Set the locators to which the writer should send periodic announcements.
     *
     * This method is used to configure the initial peers list on the PDP writer.
     *
     * @param locator_list List of locators to which the writer should send periodic announcements.
     *
     * @return true if the locators were set successfully.
     */
    void set_initial_peers(
            const LocatorList& locator_list);

    /**
     * Reset the unsent changes.
     */
    void send_periodic_announcement();

protected:

    bool send_to_fixed_locators(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const override;

private:

    /**
     * This method checks whether a CacheChange_t is relevant for the specified reader
     * This callback should return always the same result given the same arguments
     * @param change The CacheChange_t to be evaluated
     * @param reader_guid remote reader GUID_t
     * @return true if relevant, false otherwise.
     */
    bool is_relevant(
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const final;

    /**
     * @brief Mark all readers as interested.
     *
     * This method sets the flag indicating that all readers are interested in the data sent by this writer.
     * It is used to ensure that all readers are considered when sending data.
     * The flag will be reset when all the samples from this writer have been sent.
     */
    void mark_all_readers_interested();

    /**
     * @brief Mark an interested reader.
     *
     * Add the guid of a reader to the list of interested readers.
     *
     * @param reader_guid The GUID of the reader to mark as interested.
     */
    void add_interested_reader(
            const GUID_t& reader_guid);

    /**
     * @brief Unmark an interested reader.
     *
     * Remove the guid of a reader from the list of interested readers.
     *
     * @param reader_guid The GUID of the reader to mark as interested.
     */
    void remove_interested_reader(
            const GUID_t& reader_guid);

    /**
     * @brief Add all samples from this writer to the flow controller.
     */
    void reschedule_all_samples();

    //! Configured initial peers
    LocatorList initial_peers_{};
    //! The set of readers interested
    mutable ResourceLimitedVector<GUID_t> interested_readers_;
    //! Whether we have set that all destinations are interested
    mutable bool should_reach_all_destinations_ = false;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DISCOVERY_PARTICIPANT_SIMPLE__PDPSTATELESSWRITER_HPP

