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

#include <fastrtps/rtps/writer/ReaderLocator.h>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/rtps/writer/StatelessWriter.h>

#include "../participant/RTPSParticipantImpl.h"

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
        locator_info_.unicast.clear();
        locator_info_.multicast.clear();

        for (const Locator_t& locator : unicast_locators)
        {
            locator_info_.unicast.push_back(locator);
        }

        for (const Locator_t& locator : multicast_locators)
        {
            locator_info_.multicast.push_back(locator);
        }

        locator_info_.reset();
        locator_info_.enable(true);
        return true;
    }

    return false;
}

bool ReaderLocator::stop(const GUID_t& remote_guid)
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
        return true;
    }

    return false;
}

void ReaderLocator::send(CDRMessage_t* message) const
{
    if (locator_info_.remote_guid != c_Guid_Unknown)
    {
        if (locator_info_.unicast.size() > 0)
        {
            for (const Locator_t& locator : locator_info_.unicast)
            {
                owner_->sendSync(message, locator);
            }
        }
        else
        {
            for (const Locator_t& locator : locator_info_.multicast)
            {
                owner_->sendSync(message, locator);
            }
        }
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
