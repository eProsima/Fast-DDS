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

/**
 * @file Publisher.cpp
 *
 */

#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps_deprecated/publisher/PublisherImpl.h>

#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

Publisher::Publisher(PublisherImpl* pimpl) : mp_impl(pimpl)
{
    // TODO Auto-generated constructor stub
}

Publisher::~Publisher() {
    // TODO Auto-generated destructor stub
}

bool Publisher::write(void* Data) {
    logInfo(PUBLISHER,"Writing new data");
    return mp_impl->create_new_change(ALIVE,Data);
}

bool Publisher::write(void* Data, WriteParams &wparams) {
    logInfo(PUBLISHER,"Writing new data with WriteParams");
    return mp_impl->create_new_change_with_params(ALIVE, Data, wparams);
}

bool Publisher::dispose(void* Data)
{
    logInfo(PUBLISHER,"Disposing of Data");
    return mp_impl->create_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool Publisher::unregister(void* Data) {
    //Convert data to serialized Payload
    logInfo(PUBLISHER,"Unregistering of Data");
    return mp_impl->create_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool Publisher::dispose_and_unregister(void* Data) {
    //Convert data to serialized Payload
    logInfo(PUBLISHER,"Disposing and Unregistering Data");
    return mp_impl->create_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}

bool Publisher::removeAllChange(size_t* removed )
{
    logInfo(PUBLISHER,"Removing all data from history");
    return mp_impl->removeAllChange(removed);
}

bool Publisher::wait_for_all_acked(const eprosima::fastrtps::Duration_t& max_wait)
{
    logInfo(PUBLISHER,"Waiting for all samples acknowledged");
    return mp_impl->wait_for_all_acked(max_wait);
}

const GUID_t& Publisher::getGuid()
{
    return mp_impl->getGuid();
}

const PublisherAttributes& Publisher::getAttributes() const
{
    return mp_impl->getAttributes();
}

bool Publisher::updateAttributes(const PublisherAttributes& att)
{
    return mp_impl->updateAttributes(att);
}

void Publisher::get_offered_deadline_missed_status(OfferedDeadlineMissedStatus &status)
{
    mp_impl->get_offered_deadline_missed_status(status);
}

void Publisher::get_liveliness_lost_status(LivelinessLostStatus &status)
{
    mp_impl->get_liveliness_lost_status(status);
}

void Publisher::assert_liveliness()
{
    mp_impl->assert_liveliness();
}
