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

/*
 * @file ReaderLocator.cpp
 *
 */

#include <fastdds/rtps/writer/ReaderLocator.h>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/resources/AsyncWriterThread.h>
#include <fastdds/rtps/writer/StatelessWriter.h>
#include <fastdds/rtps/common/LocatorListComparisons.hpp>

#include <rtps/participant/RTPSParticipantImpl.h>
#include "rtps/RTPSDomainImpl.hpp"

namespace eprosima {
namespace fastrtps {
namespace rtps {

ReaderLocator::ReaderLocator(
        RTPSParticipantImpl* owner,
        size_t max_unicast_locators,
        size_t max_multicast_locators)
    : owner_(owner)
    , locator_info_(max_unicast_locators, max_multicast_locators)
    , expects_inline_qos_(false)
    , is_local_reader_(false)
    , local_reader_(nullptr)
    , guid_prefix_as_vector_(1u)
    , guid_as_vector_(1u)
{
}

bool ReaderLocator::start(
        const GUID_t& remote_guid,
        const ResourceLimitedVector<Locator_t>& unicast_locators,
        const ResourceLimitedVector<Locator_t>& multicast_locators,
        bool expects_inline_qos)
{
    if (locator_info_.remote_guid == c_Guid_Unknown)
    {
        expects_inline_qos_ = expects_inline_qos;
        guid_as_vector_.at(0) = remote_guid;
        guid_prefix_as_vector_.at(0) = remote_guid.guidPrefix;
        locator_info_.remote_guid = remote_guid;

        is_local_reader_ = RTPSDomainImpl::should_intraprocess_between(owner_->getGuid(), remote_guid);
        local_reader_ = nullptr;

        if (!is_local_reader_)
        {
            locator_info_.unicast = unicast_locators;
            locator_info_.multicast = multicast_locators;
        }

        locator_info_.reset();
        locator_info_.enable(true);
        return true;
    }

    return false;
}

bool ReaderLocator::update(
        const ResourceLimitedVector<Locator_t>& unicast_locators,
        const ResourceLimitedVector<Locator_t>& multicast_locators,
        bool expects_inline_qos)
{
    bool ret_val = false;

    if (expects_inline_qos_ != expects_inline_qos)
    {
        expects_inline_qos_ = expects_inline_qos;
        ret_val = true;
    }
    if (!(locator_info_.unicast == unicast_locators) ||
            !(locator_info_.multicast == multicast_locators))
    {
        if (!is_local_reader_)
        {
            locator_info_.unicast = unicast_locators;
            locator_info_.multicast = multicast_locators;
        }

        locator_info_.reset();
        locator_info_.enable(true);
        ret_val = true;
    }

    return ret_val;
}

bool ReaderLocator::stop(
        const GUID_t& remote_guid)
{
    if (locator_info_.remote_guid == remote_guid)
    {
        locator_info_.enable(false);
        locator_info_.reset();
        locator_info_.multicast.clear();
        locator_info_.unicast.clear();
        locator_info_.remote_guid = c_Guid_Unknown;
        guid_as_vector_.at(0) = c_Guid_Unknown;
        guid_prefix_as_vector_.at(0) = c_GuidPrefix_Unknown;
        expects_inline_qos_ = false;
        is_local_reader_ = false;
        local_reader_ = nullptr;
        return true;
    }

    return false;
}

bool ReaderLocator::send(
        CDRMessage_t* message,
        std::chrono::steady_clock::time_point& max_blocking_time_point) const
{
    if (locator_info_.remote_guid != c_Guid_Unknown)
    {
        if (locator_info_.unicast.size() > 0)
        {
            return owner_->sendSync(message, Locators(locator_info_.unicast.begin()),
                           Locators(locator_info_.unicast.end()), max_blocking_time_point);
        }
        else
        {
            return owner_->sendSync(message, Locators(locator_info_.multicast.begin()),
                           Locators(locator_info_.multicast.end()), max_blocking_time_point);
        }
    }

    return true;
}

RTPSReader* ReaderLocator::local_reader()
{
    if (!local_reader_)
    {
        local_reader_ = RTPSDomainImpl::find_local_reader(locator_info_.remote_guid);
    }
    return local_reader_;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
