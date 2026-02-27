/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_
#define _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_

#include <cstdint>

#include <gmock/gmock.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

const char* const grained_flow_controller_name = "GrainedFlowController";

class GrainedFlowController : public FlowController
{
public:

    GrainedFlowController(
            RTPSParticipantImpl* /* participant */,
            uint32_t /* async_index */,
            ThreadSettings /* thread_settings */,
            uint32_t minimum_bytes_per_period,
            uint64_t /* period_ms */)
        : minimum_bytes_per_period_(minimum_bytes_per_period)
    {
    }

    ~GrainedFlowController() override
    {
    }

    MOCK_METHOD(void, register_remote_reader, (
                const GUID_t& remote_reader_guid,
                const int32_t initial_bytes_per_period));

    MOCK_METHOD(void, unregister_remote_reader, (
                const GUID_t& remote_reader_guid));

    MOCK_METHOD(void, update_remote_reader_bytes_per_period, (
                const GUID_t& remote_reader_guid,
                uint32_t new_bytes_per_period));

    uint32_t get_max_payload() override
    {
        return minimum_bytes_per_period_;
    }

    MOCK_METHOD(void, init, (), (override));

    MOCK_METHOD(void, register_writer, (
                BaseWriter * writer), (override));

    MOCK_METHOD(void, unregister_writer, (
                BaseWriter * writer), (override));

    MOCK_METHOD(bool, add_new_sample, (
                BaseWriter * writer,
                CacheChange_t * change,
                const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time), (override));

    MOCK_METHOD(bool, add_old_sample, (
                BaseWriter * writer,
                CacheChange_t * change), (override));

    MOCK_METHOD(bool, remove_change, (
                CacheChange_t * change,
                const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time), (override));

private:

    uint32_t minimum_bytes_per_period_;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _RTPS_FLOWCONTROL_GRAINEDFLOWCONTROLLERIMPL_HPP_
