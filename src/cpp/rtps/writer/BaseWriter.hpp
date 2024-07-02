// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BaseWriter.hpp
 */

#ifndef RTPS_WRITER__BASEWRITER_HPP
#define RTPS_WRITER__BASEWRITER_HPP

#include <cstdint>
#include <memory>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>

#include <rtps/writer/DeliveryRetCode.hpp>
#include <rtps/writer/LocatorSelectorSender.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CacheChange_t;
class DataSharingNotifier;
class FlowController;
struct GUID_t;
class ICacheChangePool;
class IPayloadPool;
class RTPSMessageGroup;
class RTPSParticipantImpl;
class WriterAttributes;
class WriterHistory;
class WriterListener;

class BaseWriter
    : public fastdds::rtps::RTPSWriter
    , public fastdds::statistics::StatisticsWriterImpl
{

public:

#ifdef FASTDDS_STATISTICS

    /**
     * Add a listener to receive statistics backend callbacks
     * @param listener
     * @return true if successfully added
     */
    FASTDDS_EXPORTED_API bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    /**
     * Remove a listener from receiving statistics backend callbacks
     * @param listener
     * @return true if successfully removed
     */
    FASTDDS_EXPORTED_API bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    /**
     * @brief Set the enabled statistics writers mask
     *
     * @param enabled_writers The new mask to set
     */
    FASTDDS_EXPORTED_API void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) override;

#endif // FASTDDS_STATISTICS

    /**
     * Tells writer the sample can be sent to the network.
     * This function should be used by a fastdds::rtps::FlowController.
     *
     * @param cache_change Pointer to the CacheChange_t that represents the sample which can be sent.
     * @param group RTPSMessageGroup reference uses for generating the RTPS message.
     * @param locator_selector RTPSMessageSenderInterface reference uses for selecting locators. The reference has to
     * be a member of this RTPSWriter object.
     * @param max_blocking_time Future timepoint where blocking send should end.
     * @return Return code.
     * @note Must be non-thread safe.
     */
    virtual DeliveryRetCode deliver_sample_nts(
            CacheChange_t* cache_change,
            RTPSMessageGroup& group,
            LocatorSelectorSender& locator_selector,
            const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time) = 0;

    virtual LocatorSelectorSender& get_general_locator_selector() = 0;

    virtual LocatorSelectorSender& get_async_locator_selector() = 0;

    /**
     * Send a message through this interface.
     *
     * @param buffers Vector of NetworkBuffers to send with data already serialized.
     * @param total_bytes Total number of bytes to send. Should be equal to the sum of the @c size field of all buffers.
     * @param locator_selector RTPSMessageSenderInterface reference uses for selecting locators. The reference has to
     * be a member of this RTPSWriter object.
     * @param max_blocking_time_point Future timepoint where blocking send should end.
     */
    virtual bool send_nts(
            const std::vector<eprosima::fastdds::rtps::NetworkBuffer>& buffers,
            const uint32_t& total_bytes,
            const LocatorSelectorSender& locator_selector,
            std::chrono::steady_clock::time_point& max_blocking_time_point) const;

    virtual ~BaseWriter();

protected:

    BaseWriter(
            RTPSParticipantImpl* impl,
            const GUID_t& guid,
            const WriterAttributes& att,
            FlowController* flow_controller,
            WriterHistory* hist,
            WriterListener* listen = nullptr);

    void deinit();

    void add_guid(
            LocatorSelectorSender& locator_selector,
            const GUID_t& remote_guid);

    void compute_selected_guids(
            LocatorSelectorSender& locator_selector);

    void update_cached_info_nts(
            LocatorSelectorSender& locator_selector);

    void add_statistics_sent_submessage(
            CacheChange_t* change,
            size_t num_locators);

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP
