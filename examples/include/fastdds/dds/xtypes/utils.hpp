// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file utils.hpp
 */

#ifndef _FASTDDS_DDS_XTYPES_UTILS_HPP_
#define _FASTDDS_DDS_XTYPES_UTILS_HPP_

#include <iostream>
#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

enum class DynamicDataJsonFormat // TODO: change?
{
    OMG,
    EPROSIMA, // TODO: Change for something more specific
};

ReturnCode_t json_serialize(
        const traits<DynamicData>::_ref_type& data,
        std::ostream& output,
        DynamicDataJsonFormat format);

ReturnCode_t json_serialize(
        const traits<DynamicData>::_ref_type& data,
        std::string& output,
        DynamicDataJsonFormat format);

} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_UTILS_HPP_
