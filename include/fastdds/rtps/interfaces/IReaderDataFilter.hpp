// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IReaderDataFilter.hpp
 *
 */

#ifndef _FASTDDS_RTPS_IREADERDATAFILTER_HPP_
#define _FASTDDS_RTPS_IREADERDATAFILTER_HPP_

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/Guid.h>


namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Abstract class IReaderDataFilter that acts as virtual interface for data filters in ReaderProxy.
 * @ingroup RTPS_MODULE
 */
class IReaderDataFilter
{
public:

    /**
     * This method checks whether a CacheChange_t is relevant for the specified reader
     * This callback should return always the same result given the same arguments
     * @param change The CacheChange_t to be evaluated
     * @param reader_guid remote reader GUID_t
     * @return true if relevant, false otherwise.
     */
    virtual bool is_relevant(
            const fastrtps::rtps::CacheChange_t& change,
            const fastrtps::rtps::GUID_t& reader_guid) const = 0;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_IREADERDATAFILTER_HPP_ */
