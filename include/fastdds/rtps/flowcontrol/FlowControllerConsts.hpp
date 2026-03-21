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

#ifndef FASTDDS_RTPS_FLOWCONTROL__FLOWCONTROLLERCONSTS_HPP
#define FASTDDS_RTPS_FLOWCONTROL__FLOWCONTROLLERCONSTS_HPP

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

//! Name of the default flow controller.
extern FASTDDS_EXPORTED_API const char* const FASTDDS_FLOW_CONTROLLER_DEFAULT;
//! Name of the default flow controller for statistics writers.
extern FASTDDS_EXPORTED_API const char* const FASTDDS_STATISTICS_FLOW_CONTROLLER_DEFAULT;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_FLOWCONTROL__FLOWCONTROLLERCONSTS_HPP
