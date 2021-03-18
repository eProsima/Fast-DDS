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

#ifndef FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDEFS_HPP
#define FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDEFS_HPP

#include <fastrtps/fastrtps_dll.h>
#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace rtps {

/*!
 * Supported scheduler plans by a flow controller.
 *
 * A flow controller's scheduler plan takes the decision of which samples are the next ones to be sent to the network.
 * Fast-DDS flow controller supports several scheduler plans listed in this enumeration.
 */
enum class FlowControllerScheduler : int32_t
{
    //! FIFO scheduler: first written sample by user, first sample scheduled to be sent to network.
    FIFO_SCHEDULER,
    //! Round Robind scheduler: schedules one sample of each DataWriter in circular order.
    ROUND_ROBLIN_SCHEDULER,
    //! High priority scheduler: samples with highest priority are scheduled first to be sent to network.
    HIGH_PRIORITY_SCHEDULER,
    //! Priority with reservation scheduler: guarantee each DataWriter's minimum reservation of throughput.
    //! Samples not fitting the reservation are scheduled by priority.
    PRIORITY_WITH_RESERVATION_SCHEDULER
};

extern const char* const DEFAULT_FLOW_CONTROLLER_NAME;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_FLOWCONTROL_FLOWCONTROLLERDEFS_HPP
