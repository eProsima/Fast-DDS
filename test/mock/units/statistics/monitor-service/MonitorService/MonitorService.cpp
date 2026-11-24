// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of MonitorService: replaces src/cpp/statistics/rtps/monitor-service/MonitorService.cpp, for
// unit tests creating participants, which own a monitor service only when it is enabled. Only the destructor is
// defined, as no monitor service is ever created.

#include <statistics/rtps/monitor-service/MonitorService.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

MonitorService::~MonitorService()
{
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima
