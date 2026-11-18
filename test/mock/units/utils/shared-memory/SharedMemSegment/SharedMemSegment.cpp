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

// Link-seam implementation of SharedMemSegment: replaces src/cpp/utils/shared_memory/SharedMemSegment.cpp. Computing
// the extra size of the allocations always fails, as creating the temporary segment does without permissions.

#include <utils/shared_memory/SharedMemSegment.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

template<typename T, typename U>
uint32_t SharedSegment<T, U>::compute_per_allocation_extra_size(
        size_t,
        const std::string& domain_name)
{
    throw boost::interprocess::interprocess_exception(("Cannot create a segment for " + domain_name).c_str());
}

template uint32_t SharedSegment<SharedMemSegment::managed_shared_memory_type,
        SharedMemSegment::managed_shared_object_type>::compute_per_allocation_extra_size(
        size_t allocation_alignment,
        const std::string& domain_name);

template uint32_t SharedSegment<SharedFileSegment::managed_shared_memory_type,
        SharedFileSegment::managed_shared_object_type>::compute_per_allocation_extra_size(
        size_t allocation_alignment,
        const std::string& domain_name);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
