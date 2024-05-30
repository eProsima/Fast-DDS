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
 * @file RTPSReader.h
 */

#ifndef _FASTDDS_RTPS_READER_RTPSREADER_H_
#define _FASTDDS_RTPS_READER_RTPSREADER_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/Endpoint.h>
#include <fastdds/rtps/history/IChangePool.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/history/History.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Forward declarations
struct CacheChange_t;
class ReaderListener;
class RTPSParticipantImpl;
class WriterProxyData;

/**
 * Class RTPSReader, manages the reception of data from its matched writers.
 * @ingroup READER_MODULE
 */
class RTPSReader : public Endpoint
{

protected:

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            ReaderHistory* hist,
            ReaderListener* listen);

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            ReaderHistory* hist,
            ReaderListener* listen);

    RTPSReader(
            RTPSParticipantImpl* pimpl,
            const GUID_t& guid,
            const ReaderAttributes& att,
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool,
            ReaderHistory* hist,
            ReaderListener* listen);

    ~RTPSReader();

public:

    /**
     * Add a matched writer represented by its attributes.
     * @param wdata Attributes of the writer to add.
     * @return True if correctly added.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_add(
            const WriterProxyData& wdata) = 0;

    /**
     * Remove a writer represented by its attributes from the matched writers.
     * @param writer_guid GUID of the writer to remove.
     * @param removed_by_lease Whether the writer is being unmatched due to a participant drop.
     * @return True if correctly removed.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_remove(
            const GUID_t& writer_guid,
            bool removed_by_lease = false) = 0;

    /**
     * Tells us if a specific Writer is matched against this reader.
     * @param writer_guid GUID of the writer to check.
     * @return True if it is matched.
     */
    FASTDDS_EXPORTED_API virtual bool matched_writer_is_matched(
            const GUID_t& writer_guid) = 0;

    /**
     * Assert the liveliness of a matched writer.
     * @param writer GUID of the writer to assert.
     */
    FASTDDS_EXPORTED_API virtual void assert_writer_liveliness(
            const GUID_t& writer) = 0;

    /*!
     * @brief Returns there is a clean state with all Writers.
     * It occurs when the Reader received all samples sent by Writers. In other words,
     * its WriterProxies are up to date.
     * @return There is a clean state with all Writers.
     */
    FASTDDS_EXPORTED_API virtual bool isInCleanState() = 0;

    /**
     * Get the associated listener, secondary attached Listener in case it is of compound type
     * @return Pointer to the associated reader listener.
     */
    FASTDDS_EXPORTED_API ReaderListener* getListener() const;

    /**
     * Switch the ReaderListener kind for the Reader.
     * If the RTPSReader does not belong to the built-in protocols it switches out the old one.
     * If it belongs to the built-in protocols, it sets the new ReaderListener callbacks to be called after the
     * built-in ReaderListener ones.
     * @param target Pointed to ReaderLister to attach
     * @return True is correctly set.
     */
    FASTDDS_EXPORTED_API bool setListener(
            ReaderListener* target);

    /**
     * Read the next unread CacheChange_t from the history
     * @return A pointer to the first unread CacheChange_t from the history.
     */
    FASTDDS_EXPORTED_API virtual CacheChange_t* nextUnreadCache() = 0;

    /**
     * Get the next CacheChange_t from the history to take.
     * @return A pointer to the first CacheChange_t in the history.
     */
    FASTDDS_EXPORTED_API virtual CacheChange_t* nextUntakenCache() = 0;

    FASTDDS_EXPORTED_API virtual bool wait_for_unread_cache(
            const eprosima::fastrtps::Duration_t& timeout) = 0;

    FASTDDS_EXPORTED_API virtual uint64_t get_unread_count() const = 0;

    FASTDDS_EXPORTED_API virtual uint64_t get_unread_count(
            bool mark_as_read) = 0;

    /**
     * @return True if the reader expects Inline QOS.
     */
    FASTDDS_EXPORTED_API inline bool expectsInlineQos()
    {
        return m_expectsInlineQos;
    }

    //! Returns a pointer to the associated History.
    FASTDDS_EXPORTED_API inline ReaderHistory* getHistory()
    {
        return mp_history;
    }

    //! @return The content filter associated to this reader.
    FASTDDS_EXPORTED_API eprosima::fastdds::rtps::IReaderDataFilter* get_content_filter() const
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        return data_filter_;
    }

    //! Set the content filter associated to this reader.
    //! @param filter Pointer to the content filter to associate to this reader.
    FASTDDS_EXPORTED_API void set_content_filter(
            eprosima::fastdds::rtps::IReaderDataFilter* filter)
    {
        std::lock_guard<RecursiveTimedMutex> guard(mp_mutex);
        data_filter_ = filter;
    }

    FASTDDS_EXPORTED_API inline void enableMessagesFromUnkownWriters(
            bool enable)
    {
        m_acceptMessagesFromUnkownWriters = enable;
    }

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

    //!ReaderHistory
    ReaderHistory* mp_history;
    //!Listener
    ReaderListener* mp_listener;
    //!Accept msg from unknown writers (BE-true,RE-false)
    bool m_acceptMessagesFromUnkownWriters;
    //!Expects Inline Qos.
    bool m_expectsInlineQos;

    eprosima::fastdds::rtps::IReaderDataFilter* data_filter_ = nullptr;

private:

    RTPSReader& operator =(
            const RTPSReader&) = delete;

    void init(
            const std::shared_ptr<IPayloadPool>& payload_pool,
            const std::shared_ptr<IChangePool>& change_pool);

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif /* _FASTDDS_RTPS_READER_RTPSREADER_H_ */
