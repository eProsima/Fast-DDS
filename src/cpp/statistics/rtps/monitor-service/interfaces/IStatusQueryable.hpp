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
#include <fastdds/rtps/common/Guid.hpp>

#include <statistics/types/monitorservice_types.hpp>

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
     * @param [in] guid The GUID_t identifying the target entity
     * @param [in] status_kind The monitor service status kind that has changed
     * @param [out] status The requested entity status
     * @return Whether the operation succeeded or not
     */
    virtual bool get_monitoring_status(
            const fastdds::rtps::GUID_t& guid,
            MonitorServiceData& status) = 0;

};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSQUERYABLE_HPP_

