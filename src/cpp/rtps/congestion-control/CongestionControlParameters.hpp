/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLPARAMETERS_HPP_
#define RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLPARAMETERS_HPP_

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * @brief Structure to hold congestion control parameters
 */
struct CongestionControlParameters
{
    //! Period duration in milliseconds for congestion control updates
    uint32_t period_duration_ms = 10000;
    /*!
     * Initial target number of bytes per second. Initially, if value is 0, it set automatically to use default initial
     * bandwidth (half the size of the socket send buffer), but allowing for at least 1 message per second.
     */
    uint32_t initial_target_bytes_per_second = 0;
    //! Increase factor for congestion control
    float increase_factor = 1.2f;
    //! Decrease factor for congestion control
    float decrease_factor = 0.75f;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLPARAMETERS_HPP_
