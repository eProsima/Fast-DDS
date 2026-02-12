/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <rtps/transport/ethernet/InputChannelManager.hpp>

#if defined(__linux__)

#include "./linux/InputChannelManager.ipp"

#else

#include "./empty/InputChannelManager.ipp"

#endif  // if defined(__linux__)
