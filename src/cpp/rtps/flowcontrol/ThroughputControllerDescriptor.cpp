// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/rtps/flowcontrol/ThroughputControllerDescriptor.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

ThroughputControllerDescriptor::ThroughputControllerDescriptor(): bytesPerPeriod(UINT32_MAX), periodMillisecs(0)
{
}

ThroughputControllerDescriptor::ThroughputControllerDescriptor(uint32_t size, uint32_t time): bytesPerPeriod(size), periodMillisecs(time)
{
}

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima
