/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#ifndef SRC_CPP_RTPS_NETWORK__IPCHANGEMONITORIMPL_HPP
#define SRC_CPP_RTPS_NETWORK__IPCHANGEMONITORIMPL_HPP

#if defined(_WIN32)

#include "windows/IPChangeMonitorImpl.ipp"

#elif defined(__linux__)

#include "linux/IPChangeMonitorImpl.ipp"

#else

namespace eprosima {
namespace fastdds {
namespace rtps {

struct IPChangeMonitorImpl::IPChangeMonitorImplData
{
};

IPChangeMonitorImpl::IPChangeMonitorImpl()
{
    // Default constructor does nothing
}

void IPChangeMonitorImpl::start_monitoring()
{
    // Default implementation does nothing
}

void IPChangeMonitorImpl::stop_monitoring()
{
    // Default implementation does nothing
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif  // defined(_WIN32)

#endif  // SRC_CPP_RTPS_NETWORK__IPCHANGEMONITORIMPL_HPP
