// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file ResourceLimitedContainerConfig.hpp
 *
 */

#ifndef FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDCONTAINERCONFIG_HPP_
#define FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDCONTAINERCONFIG_HPP_

#include <limits>

namespace eprosima {
namespace fastrtps {

/**
 * Specifies the configuration of a resource limited collection.
 * @ingroup UTILITIES_MODULE
 */
struct ResourceLimitedContainerConfig
{

#if defined(_MSC_VER) && _MSC_VER <= 1900

    // Initializer list construction won't work on MSVS 2015 if you have struct fields initializers
    ResourceLimitsConfig(
            size_t ini = 0, 
            size_t max = std::numeric_limits<size_t>::max(),
            size_t inc = 1u)
        : initial(ini)
        , maximum(max)
        , increment(inc)
    {
    }

    // MSVS 2015 is not able to treat our static inline methods as constexpr
    #define ___CONSTEXPR___
#else
    #define ___CONSTEXPR___ constexpr
#endif

    //! Number of elements to be preallocated in the collection.
    size_t initial = 0;
    //! Maximum number of elements allowed in the collection.
    size_t maximum = std::numeric_limits<size_t>::max();
    //! Number of items to add when capacity limit is reached.
    size_t increment = 1u;

    /**
     * Return a resource limits configuration for a fixed size collection.
     * @param size Number of elements to allocate.
     * @return Resource limits configuration.
     */
    inline static ___CONSTEXPR___
    ResourceLimitedContainerConfig fixed_size_configuration(size_t size)
    {
        return { size, size, 0 };
    }

    /**
     * Return a resource limits configuration for a linearly growing, dynamically allocated collection.
     * @param increment Number of new elements to allocate when increasing the capacity of the collection.
     * @return Resource limits configuration.
     */
    inline static ___CONSTEXPR___
    ResourceLimitedContainerConfig dynamic_allocation_configuration(size_t increment = 1u)
    {
        return { 0, std::numeric_limits<size_t>::max(), increment ? increment : 1u };
    }

};

}  // namespace fastrtps
}  // namespace eprosima

#endif /* FASTRTPS_UTILS_COLLECTIONS_RESOURCELIMITEDCONTAINERCONFIG_HPP_ */
