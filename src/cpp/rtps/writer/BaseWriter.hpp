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

    void add_statistics_sent_submessage(
            CacheChange_t* change,
            size_t num_locators);

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_WRITER__BASEWRITER_HPP
