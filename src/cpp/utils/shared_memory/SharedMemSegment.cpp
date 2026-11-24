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

#include <utils/shared_memory/SharedMemSegment.hpp>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

template<typename T, typename U>
uint32_t SharedSegment<T, U>::compute_per_allocation_extra_size(
        size_t allocation_alignment,
        const std::string& domain_name)
{
    Id uuid;

    try
    {
        static uint32_t extra_size = 0;

        if (extra_size == 0)
        {
            uuid.generate();

            auto name = domain_name + "_" + uuid.to_string();

            SharedMemEnvironment::get().init();

            {
                managed_shared_memory_type
                        test_segment(boost::interprocess::create_only, name.c_str(),
                        (std::max)((uint32_t)1024, static_cast<uint32_t>(allocation_alignment * 4)));

                auto m1 = test_segment.get_free_memory();
                test_segment.allocate_aligned(1, static_cast<uint32_t>(allocation_alignment));
                auto m2 = test_segment.get_free_memory();
                extra_size = static_cast<uint32_t>(m1 - m2);
            }

            managed_shared_object_type::remove(name.c_str());
        }

        return extra_size;

    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(RTPS_TRANSPORT_SHM, "Failed to create segment " << uuid.to_string()
                                                                           << ": " << e.what());

        throw;
    }
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
