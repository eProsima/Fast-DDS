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

/**
 * @file LocatorsIterator.hpp
 */

#ifndef _FASTDDS_RTPS_COMMON_LOCATORSITERATOR_HPP_
#define _FASTDDS_RTPS_COMMON_LOCATORSITERATOR_HPP_

#include <fastdds/rtps/common/Locator.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Provides a Locator's iterator interface that can be used by different Locator's
 * containers
 */
struct LocatorsIterator
{
    virtual LocatorsIterator& operator ++() = 0;
    virtual bool operator ==(
            const LocatorsIterator& other) const = 0;
    virtual bool operator !=(
            const LocatorsIterator& other) const = 0;
    virtual const Locator& operator *() const = 0;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_RTPS_COMMON_LOCATORSITERATOR_HPP_ */
