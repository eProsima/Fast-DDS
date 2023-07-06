// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IStatusQueryable.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSQUERYABLE_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSQUERYABLE_HPP_

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/rtps/common/Guid.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IStatusQueryable
{
    /**
    * @brief Interface for requesting the IncompatibleQosStatus
    * of and entity identified by its guid.
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_incompatible_qos_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::IncompatibleQosStatus& status) = 0;

    /**
    * @brief Interface for requesting the InconsistentTopicStatus
    * of and entity identified by its guid.
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_inconsistent_topic_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::InconsistentTopicStatus& status) = 0;

    /**
    * @brief Interface for requesting the LivelinessLostStatus
    * of a writer identified by its guid.
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_liveliness_lost_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::LivelinessLostStatus& status) = 0;

    /**
    * @brief Interface for requesting the LivelinessChangedStatus
    * of a reader identified by its guid.
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_liveliness_changed_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::LivelinessChangedStatus& status) = 0;

    /**
    * @brief Interface for requesting the DeadlineMissedStatus
    * of an entity identified by its guid.
    * [offered] for the writer
    * [requested] for the reader
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_deadline_missed_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::DeadlineMissedStatus& status) = 0;

    /**
    * @brief Interface for requesting the SampleLostStatus
    * of a reader identified by its guid.
    *
    * @param[in] guid The GUID_t identifying the target entity
    * @param[out] status The requested entity status
    * @return Whether the operation succeeded or not
    */
    virtual bool get_sample_lost_status(
            const fastrtps::rtps::GUID_t& guid,
            dds::SampleLostStatus& status) = 0;

};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSQUERYABLE_HPP_

