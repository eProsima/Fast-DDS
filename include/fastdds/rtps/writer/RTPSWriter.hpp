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
 * @file RTPSWriter.hpp
 */

#ifndef FASTDDS_RTPS_WRITER__RTPSWRITER_HPP
#define FASTDDS_RTPS_WRITER__RTPSWRITER_HPP

#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class FlowController;
class RTPSParticipantImpl;
class WriterHistory;
class WriterListener;

/**
 * Class RTPSWriter, manages the sending of data to the readers. Is always associated with a HistoryCache.
 * @ingroup WRITER_MODULE
 */
class RTPSWriter : public Endpoint
{
protected:

    RTPSWriter(
            RTPSParticipantImpl* impl,
            const GUID_t& guid,
            const WriterAttributes& att);

    virtual ~RTPSWriter();

public:

    /**
     * @brief Add a matched reader represented by its attributes.
     *
     * @param info  Subscription info of the reader being matched.
     *
     * @return True if added.
     */
    FASTDDS_EXPORTED_API virtual bool matched_reader_add(
            const SubscriptionBuiltinTopicData& info) = 0;

    /**
     * Remove a matched reader.
     * @param reader_guid GUID of the reader to remove.
     * @return True if removed.
     */
    FASTDDS_EXPORTED_API virtual bool matched_reader_remove(
            const GUID_t& reader_guid) = 0;

    /**
     * Tells us if a specific Reader is matched against this writer.
     * @param reader_guid GUID of the reader to check.
     * @return True if it was matched.
     */
    FASTDDS_EXPORTED_API virtual bool matched_reader_is_matched(
            const GUID_t& reader_guid) = 0;

    /**
     * @brief Set a content filter to perform content filtering on this writer.
     *
     * This method sets a content filter that will be used to check whether a cache change is relevant
     * for a reader or not.
     *
     * @param filter  The content filter to use on this writer. May be @c nullptr to remove the content filter
     *                (i.e. treat all samples as relevant).
     */
    FASTDDS_EXPORTED_API virtual void reader_data_filter(
            IReaderDataFilter* filter) = 0;

    /**
     * @brief Get the content filter used to perform content filtering on this writer.
     *
     * @return The content filter used on this writer.
     */
    FASTDDS_EXPORTED_API virtual const IReaderDataFilter* reader_data_filter() const = 0;

    /**
     * @brief Check if a specific change has been delivered to the transport layer of every matched remote RTPSReader
     * at least once.
     *
     * @param seq_num Sequence number of the change to check.
     * @return true if delivered. False otherwise.
     */
    FASTDDS_EXPORTED_API virtual bool has_been_fully_delivered(
            const SequenceNumber_t& seq_num) const = 0;

    /**
     * Check if a specific change has been acknowledged by all Readers.
     * Is only useful in reliable Writer. In BE Writers returns false when pending to be sent.
     *
     * @param seq_num Sequence number to check.
     *
     * @return True if acknowledged by all.
     */
    FASTDDS_EXPORTED_API virtual bool is_acked_by_all(
            const SequenceNumber_t& seq_num) const = 0;

    /**
     * Waits until all changes were acknowledged or max_wait.
     *
     * @param max_wait Maximum time to wait.
     *
     * @return True if all were acknowledged.
     */
    FASTDDS_EXPORTED_API virtual bool wait_for_all_acked(
            const dds::Duration_t& max_wait) = 0;

    /**
     * Update the Attributes of the Writer.
     * @param att New attributes
     */
    FASTDDS_EXPORTED_API virtual void update_attributes(
            const WriterAttributes& att) = 0;

    /**
     * Get listener
     * @return Listener
     */
    FASTDDS_EXPORTED_API virtual WriterListener* get_listener() const = 0;

    /**
     * Set the listener.
     *
     * @param listener Pointer to the listener.
     * @return True if correctly set.
     */
    FASTDDS_EXPORTED_API virtual bool set_listener(
            WriterListener* listener) = 0;

    /**
     * Get the publication mode
     * @return publication mode
     */
    FASTDDS_EXPORTED_API virtual bool is_async() const = 0;

    /**
     * @brief Returns if disable positive ACKs QoS is enabled.
     *
     * @return Best effort writers always return false.
     *         Reliable writers override this method.
     */
    FASTDDS_EXPORTED_API virtual bool get_disable_positive_acks() const = 0;

    /**
     * @brief Fills the provided vector with the GUIDs of the matched readers.
     *
     * @param[out] guids Vector to be filled with the GUIDs of the matched readers.
     * @return True if the operation was successful.
     */
    FASTDDS_EXPORTED_API virtual bool matched_readers_guids(
            std::vector<GUID_t>& guids) const = 0;

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
     * @brief Get the connection list of this writer
     *
     * @param [out] connection_list of the writer
     * @return True if could be retrieved
     */
    FASTDDS_EXPORTED_API virtual bool get_connections(
            fastdds::statistics::rtps::ConnectionList& connection_list) = 0;

#endif // FASTDDS_STATISTICS

private:

    RTPSWriter& operator =(
            const RTPSWriter&) = delete;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__RTPSWRITER_HPP
