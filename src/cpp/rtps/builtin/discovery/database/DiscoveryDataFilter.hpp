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
 * @file DiscoveryDataFilter.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_DATA_FILTER_H_
#define _FASTDDS_RTPS_DISCOVERY_DATA_FILTER_H_

#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to filter PDP data depending on the destination reader.
 *    1. Template parameter <DiscoveryDataBase> represents the DiscoveryDataBase
 *@ingroup DISCOVERY_MODULE
 */
template<class DiscoveryDataBase>
class PDPDataFilter
    : public IReaderDataFilter
{
public:

    bool is_relevant(
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const override
    {
        return static_cast<const DiscoveryDataBase*>(this)->pdp_is_relevant(change, reader_guid);
    }

};

/**
 * Class to filter EDP data depending on the destination reader.
 *    1. Template parameter <DiscoveryDataBase> represents the DiscoveryDataBase
 *    2. Template parameter <publications> represents whether the class is specialized for publications or
 *       subscriptions data [Default to publications].
 *@ingroup DISCOVERY_MODULE
 */
template<class DiscoveryDataBase, bool publications = true>
class EDPDataFilter
    : public IReaderDataFilter
{
public:

    bool is_relevant(
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const override
    {
        return static_cast<const DiscoveryDataBase*>(this)->edp_publications_is_relevant(change, reader_guid);
    }

};

/**
 * Class to filter EDP subscriptions data depending on the destination reader.
 *@ingroup DISCOVERY_MODULE
 */
template<class DiscoveryDataBase>
class EDPDataFilter<DiscoveryDataBase, false>
    : public IReaderDataFilter
{
public:

    bool is_relevant(
            const fastdds::rtps::CacheChange_t& change,
            const fastdds::rtps::GUID_t& reader_guid) const override
    {
        return static_cast<const DiscoveryDataBase*>(this)->edp_subscriptions_is_relevant(change, reader_guid);
    }

};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_DATA_FILTER_H_ */
