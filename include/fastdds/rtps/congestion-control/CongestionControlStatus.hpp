/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file CongestionControlStatus.hpp
 */

#ifndef FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLSTATUS_HPP
#define FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLSTATUS_HPP

#include <cstdint>

#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//! Direction of a per-reader bandwidth-limit change.
enum class CongestionControlLimitDirection : int32_t
{
    //! The limit was raised.
    INCREASE = 0,
    //! The limit was lowered.
    DECREASE = 1
};

/**
 * @brief Status carried by CongestionControlListener::on_cc_limit_update.
 *
 * Emitted by every congestion-control algorithm whenever the bandwidth limit of a
 * remote reader changes.
 */
struct CongestionControlLimitUpdateStatus
{
    //! Remote reader whose limit changed.
    GUID_t reader_guid{};
    //! Limit (bytes per period) before this update.
    uint32_t previous_limit = 0;
    //! Limit (bytes per period) after this update.
    uint32_t new_limit = 0;
    //! Whether the limit went up or down.
    CongestionControlLimitDirection direction = CongestionControlLimitDirection::INCREASE;
};

/**
 * @brief Status carried by CongestionControlListener::on_cc_status_check.
 *
 * Periodic per-reader snapshot, emitted on every evaluation cycle regardless of whether
 * the limit changed.
 */
struct CongestionControlStatus
{
    //! Remote reader being evaluated.
    GUID_t reader_guid{};
    //! Current bandwidth limit (bytes per period) at the end of this cycle.
    uint32_t current_limit = 0;
    //! Bytes first-time-sent during the cycle (algorithms that do not track sends report 0).
    uint64_t period_sent_bytes = 0;
    //! Bytes acknowledged during the cycle.
    uint64_t period_acked_bytes = 0;
    //! Bytes counted as resent during the cycle.
    uint64_t period_resent_bytes = 0;
    //! Delivered throughput (acked bytes / elapsed seconds) over the cycle.
    uint64_t acked_bps = 0;
    //! Nominal evaluation period in milliseconds.
    uint32_t period_duration_ms = 0;
};

/**
 * @brief Status carried by CongestionControlListener::on_cc_info.
 *
 * Generic algorithm-specific channel. Each algorithm assigns its own meaning to the
 * @c code / @c value pair (see CongestionControlInfoCodes.hpp).
 * A single pair is emitted for every callback to simplify the listener interface and
 * reduce latency between the status update and the info.
 */
struct CongestionControlInfoStatus
{
    //! Remote reader the info pertains to.
    GUID_t reader_guid{};
    //! Algorithm-specific code identifying the meaning of @c value.
    int32_t code = 0;
    //! Algorithm-specific value, interpreted relative to @c code.
    int32_t value = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLSTATUS_HPP
