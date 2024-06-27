// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocatorWithMask.hpp
 */

#ifndef FASTDDS_RTPS_COMMON__LOCATORWITHMASK_HPP
#define FASTDDS_RTPS_COMMON__LOCATORWITHMASK_HPP

#include <sstream>

#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/Locator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * A Locator with a mask that defines the number of significant bits of its address.
 */
class FASTDDS_EXPORTED_API LocatorWithMask : public Locator
{
public:

    /**
     * Get the number of significant bits on the address of this locator.
     *
     * @return number of significant bits on the address of this locator.
     */
    uint8_t mask() const;

    /**
     * Set the number of significant bits on the address of this locator.
     *
     * @param mask number of significant bits on the address of this locator.
     */
    void mask(
            uint8_t mask);

    /**
     * Check whether the given locator is from the same network as this locator.
     *
     * @param loc locator to check if belonging to the same network as this locator.
     *
     * @return true if the two locators are from the same network, false otherwise.
     */
    bool matches(
            const Locator& loc) const;

    //! Copy assignment
    LocatorWithMask& operator =(
            const Locator& loc);

private:

    uint8_t mask_ = 24;
};

FASTDDS_EXPORTED_API std::ostream& operator <<(
        std::ostream& output,
        const LocatorWithMask& loc);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_COMMON__LOCATORWITHMASK_HPP
