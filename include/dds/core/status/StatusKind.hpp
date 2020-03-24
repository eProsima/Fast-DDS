/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OMG_DDS_CORE_STATUS_STATUSKIND_HPP_
#define OMG_DDS_CORE_STATUS_STATUSKIND_HPP_

#include <dds/core/SafeEnumeration.hpp>
#include <dds/core/conformance.hpp>

#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>

namespace dds {
namespace core {
namespace status {

struct SampleRejectedKind_def
{
    enum Type
    {
        NOT_REJECTED,                              //!< No sample has been rejected yet.
        REJECTED_BY_INSTANCES_LIMIT,               //!< Rejected by instance limit.
        REJECTED_BY_SAMPLES_LIMIT,                 //!< Rejected by samples limit.
        REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT     //!< Rejected by samples per instance limit.
    };
};

typedef dds::core::SafeEnum<SampleRejectedKind_def> SampleRejectedKind;

} //namespace status
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_STATUS_STATUSKIND_HPP_
