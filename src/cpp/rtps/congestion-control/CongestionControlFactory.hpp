/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLFACTORY_HPP_
#define RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLFACTORY_HPP_

#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ICongestionControl;

/**
 * @brief A factory of Congestion Control implementations.
 */
class CongestionControlFactory
{
public:

    static constexpr const char* BASIC_CONGESTION_CONTROL = "basic";
    static constexpr const char* DEFAULT_CONGESTION_CONTROL = BASIC_CONGESTION_CONTROL;

    /**
     * @brief Create a congestion control implementation by name.
     *
     * @param name  Name of the congestion control implementation.
     *
     * @return Pointer to the created ICongestionControl instance, or nullptr if not found.
     */
    static ICongestionControl* create_congestion_control(
            const std::string& name);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // RTPS_CONGESTION_CONTROL__CONGESTIONCONTROLFACTORY_HPP_
