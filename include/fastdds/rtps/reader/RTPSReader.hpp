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
 * @file RTPSReader.hpp
 */

#ifndef FASTDDS_RTPS_READER__RTPSREADER_HPP
#define FASTDDS_RTPS_READER__RTPSREADER_HPP

#include <cstdint>
#include <memory>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

// Forward declarations
struct CacheChange_t;
class ReaderListener;
class RTPSParticipantImpl;

/**
 * Class RTPSReader, manages the reception of data from its matched writers.
 * Needs to be constructed using the createRTPSReader method from the RTPSDomain.
 * @ingroup READER_MODULE
 */
class RTPSReader : public Endpoint
{

public:

    /**
     * @brief Add a matched writer represented by its publication info.
     *
     * @param info  Publication info of the writer being matched.
     *
     * @return True if correctly added.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_add(
            const PublicationBuiltinTopicData& info) = 0;

    /**
     * @brief Remove a writer from the matched writers.
     *
     * @param writer_guid       GUID of the writer to remove.
     * @param removed_by_lease  Whether the writer is being unmatched due to a participant drop.
     *
     * @return True if correctly removed.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_remove(
            const GUID_t& writer_guid,
            bool removed_by_lease = false) = 0;

    /**
     * @brief Check if a specific writer is matched against this reader.
     *
     * @param writer_guid  GUID of the writer to check.
     *
     * @return True if the specified writer is matched with this reader.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_is_matched(
            const GUID_t& writer_guid) = 0;

    /**
     * @brief Assert the liveliness of a matched writer.
     *
     * @param writer  GUID of the writer on which to assert liveliness.
     */
    FASTDDS_EXPORTED_API virtual void assert_writer_liveliness(
            const GUID_t& writer) = 0;

    /**
     * @brief Check if this reader is in a clean state with all its matched writers.
     * This will happen when the reader has received all samples announced by all its matched writers.
     *
     * @return Whether the reader is in a clean state with all its matched writers.
     */
    FASTDDS_EXPORTED_API virtual bool is_in_clean_state() = 0;

    /**
     * @brief Get the associated listener.
     *
     * @return Pointer to the associated reader listener.
     */
    FASTDDS_EXPORTED_API virtual ReaderListener* get_listener() const = 0;

    /**
     * @brief Change the listener associated to this reader.
     *
     * @param listener  The new listener to associate to this reader.
     */
    FASTDDS_EXPORTED_API virtual void set_listener(
            ReaderListener* listener) = 0;

    /**
     * @return True if the reader expects Inline QoS.
     */
    FASTDDS_EXPORTED_API virtual bool expects_inline_qos() const = 0;

    /**
     * @return a pointer to the associated History.
     */
    FASTDDS_EXPORTED_API virtual ReaderHistory* get_history() const = 0;

    /**
     * @return The content filter associated to this reader.
     */
    FASTDDS_EXPORTED_API virtual eprosima::fastdds::rtps::IReaderDataFilter* get_content_filter() const = 0;

    /**
     * Set the content filter associated to this reader.
     *
     * @param filter  Pointer to the content filter to associate to this reader.
     */
    FASTDDS_EXPORTED_API virtual void set_content_filter(
            eprosima::fastdds::rtps::IReaderDataFilter* filter) = 0;

    /**
     * @brief Fills the provided vector with the GUIDs of the matched writers.
     *
     * @param[out] guids Vector to be filled with the GUIDs of the matched writers.
     * @return True if the operation was successful.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writers_guids(
            std::vector<GUID_t>& guids) const = 0;

    /**
     * @brief Read the next unread CacheChange_t from the history.
     *
     * @return A pointer to the first unread CacheChange_t from the history.
     */
    FASTDDS_EXPORTED_API virtual CacheChange_t* next_unread_cache() = 0;

    /**
     * @brief Get the next CacheChange_t from the history to take.
     *
     * @return A pointer to the first CacheChange_t in the history.
     */
    FASTDDS_EXPORTED_API virtual CacheChange_t* next_untaken_cache() = 0;

    /**
     * Wait until there is an unread CacheChange_t in the history.
     *
     * @param timeout  Maximum time to wait.
     *
     * @return true if there is an unread CacheChange_t in the history.
     */
    FASTDDS_EXPORTED_API virtual bool wait_for_unread_cache(
            const eprosima::fastdds::dds::Duration_t& timeout) = 0;

    /**
     * Get the number of unread CacheChange_t in the history.
     *
     * @return The number of unread CacheChange_t in the history.
     */
    FASTDDS_EXPORTED_API virtual uint64_t get_unread_count() const = 0;

    /**
     * Get the number of unread CacheChange_t in the history and optionally mark them as read.
     *
     * @param mark_as_read  Whether to mark the unread CacheChange_t as read.
     *
     * @return The number of previously unread CacheChange_t in the history.
     */
    FASTDDS_EXPORTED_API virtual uint64_t get_unread_count(
            bool mark_as_read) = 0;

    /**
     * Checks whether the sample is still valid or is corrupted.
     *
     * @param data    Pointer to the sample data to check.
     *                If it does not belong to the payload pool passed to the
     *                reader on construction, it yields undefined behavior.
     * @param writer  GUID of the writer that sent \c data.
     * @param sn      Sequence number related to \c data.
     *
     * @return true if the sample is valid
     */
    FASTDDS_EXPORTED_API virtual bool is_sample_valid(
            const void* data,
            const GUID_t& writer,
            const SequenceNumber_t& sn) const = 0;

#ifdef FASTDDS_STATISTICS

    /**
     * Add a listener to receive statistics backend callbacks
     * @param listener
     * @return true if successfully added
     */
    FASTDDS_EXPORTED_API virtual bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) = 0;

    /**
     * Remove a listener from receiving statistics backend callbacks
     * @param listener
     * @return true if successfully removed
     */
    FASTDDS_EXPORTED_API virtual bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) = 0;

    /**
     * @brief Set the enabled statistics writers mask
     *
     * @param enabled_writers The new mask to set
     */
    FASTDDS_EXPORTED_API virtual void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) = 0;

    /**
     * @brief Get the connection list of this reader
     *
     * @param [out] connection_list of the reader
     * @return True if could be retrieved
     */
    FASTDDS_EXPORTED_API virtual bool get_connections(
            fastdds::statistics::rtps::ConnectionList& connection_list) = 0;

#endif // FASTDDS_STATISTICS

protected:

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist);

    ~RTPSReader();

    /// ReaderHistory
    ReaderHistory* history_;

private:

    RTPSReader& operator =(
            const RTPSReader&) = delete;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__RTPSREADER_HPP
