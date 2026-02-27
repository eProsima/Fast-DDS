/* Copyright(C) 2026, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <rtps/congestion-control/CongestionControlFactory.hpp>

#include <string>

#include <rtps/congestion-control/ICongestionControl.hpp>
#include <rtps/congestion-control/basic/CongestionControlBasic.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

ICongestionControl* CongestionControlFactory::create_congestion_control(
        const std::string& name)
{
    if (name == BASIC_CONGESTION_CONTROL)
    {
        return new CongestionControlBasic();
    }
    return nullptr;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
