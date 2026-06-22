/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file CongestionControlInfoCodes.hpp
 *
 * Documented meaning of the algorithm-specific @c {code, value} pairs delivered through
 * CongestionControlListener::on_cc_info. The listener interface itself is algorithm
 * agnostic (it only knows @c int32_t code/value); adding a new algorithm only requires
 * declaring its codes here, with no change to the listener interface.
 */

#ifndef FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLINFOCODES_HPP
#define FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLINFOCODES_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

//! @c on_cc_info codes used by the "basic" congestion-control algorithm.
enum class CongestionControlBasicInfoCode : int32_t
{
    //! value = bytes acknowledged during the period.
    ACKS_RECEIVED          = 1,
    //! value = delivered throughput as a percentage of the current limit.
    BANDWIDTH_USED_PERCENT = 2,
    //! value = bytes resent during the period.
    RESENT_BYTES           = 3
};

//! @c on_cc_info codes used by the "reno" congestion-control algorithm.
enum class CongestionControlRenoInfoCode : int32_t
{
    //! value = the new state, one of CongestionControlRenoState.
    STATE_TRANSITION = 1
};

//! Reno state-machine states reported as the @c value of a STATE_TRANSITION info.
//! Kept independent from the internal RenoState so the internal enum can evolve
//! without breaking this public ABI.
enum class CongestionControlRenoState : int32_t
{
    SLOW_START           = 1,
    FAST_RECOVERY        = 2,
    CONGESTION_AVOIDANCE = 3,
    TIMEOUT_RECOVERY     = 4
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLINFOCODES_HPP
