// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDESCRIPTOR_HPP
#define FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDESCRIPTOR_HPP

#include "FlowControllerDefs.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

/*!
 * Configuration values for creating flow controllers.
 *
 * This descriptor is used to define the configuration applied in the creation of a flow controller.
 */
struct FlowControllerDescriptor
{
    //! Name of the flow controller.
    const char* name = nullptr;

    //! Scheduler plan used by the flow controller.
    //!
    //! Default value: FlowControllerScheduler::FIFO_SCHEDULER
    FlowControllerScheduler scheduler = FlowControllerScheduler::FIFO_SCHEDULER;

    //! Maximum number of bytes to be sent to network per period.
    //!
    //! Range of bytes: [1, 2147483647];
    //! 0 value means no limit.
    //! Default value: 0
    int32_t max_bytes_per_period = 0;

    //! Period time in milliseconds.
    //!
    //! Default value: 100ms.
    uint64_t period_ms = 100;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDESCRIPTOR_HPP
