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

#include <iostream>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include "dynamic_types/DynamicDataImpl.hpp"

#include "serializers/json/dynamic_data_json.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        const DynamicDataJsonFormat format,
        std::ostream& output) noexcept
{
    ReturnCode_t ret;
    nlohmann::json j;
    if (RETCODE_OK == (ret = json_serialize(traits<DynamicData>::narrow<DynamicDataImpl>(data), j, format)))
    {
        output << j;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while performing DynamicData to JSON serialization.");
    }
    return ret;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
