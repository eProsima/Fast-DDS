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
 * @file DataWriterQos.cpp
 *
 */

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

using namespace eprosima::fastdds::dds;

const DataWriterQos eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;

bool DataWriterQos::check_qos() const
{
    if (durability_.kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(RTPS_QOS_CHECK, "PERSISTENT Durability not supported");
        return false;
    }
    if (destination_order_.kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(RTPS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return false;
    }
    if (reliability_.kind == BEST_EFFORT_RELIABILITY_QOS && ownership_.kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(RTPS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return false;
    }
    if (liveliness_.kind == AUTOMATIC_LIVELINESS_QOS || liveliness_.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (liveliness_.lease_duration < fastrtps::c_TimeInfinite &&
                liveliness_.lease_duration <= liveliness_.announcement_period)
        {
            logError(RTPS_QOS_CHECK, "WRITERQOS: LeaseDuration <= announcement period.");
            return false;
        }
    }
    return true;
}
