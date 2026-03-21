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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include "dynamic_types/DynamicDataImpl.hpp"
#include "serializers/idl/dynamic_type_idl.hpp"
#include "serializers/json/dynamic_data_json.hpp"
#include "serializers/json/json_dynamic_data.hpp"
#include "utils/collections/TreeNode.hpp"

#include <iostream>


namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t idl_serialize(
        const DynamicType::_ref_type& dynamic_type,
        std::ostream& output) noexcept
{
    ReturnCode_t ret = RETCODE_OK;

    // Create a tree representation of the dynamic type
    utilities::collections::TreeNode<TreeNodeType> root;
    ret = dyn_type_to_tree(dynamic_type, "ROOT", root);

    if (ret != RETCODE_OK)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Failed to convert DynamicType to tree.");
        return ret;
    }

    // Serialize the tree to IDL
    ret = dyn_type_tree_to_idl(root, output);

    if (ret != RETCODE_OK)
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Failed to convert DynamicType tree to IDL.");
        return ret;
    }

    return RETCODE_OK;
}

ReturnCode_t json_serialize(
        const DynamicData::_ref_type& data,
        const DynamicDataJsonFormat format,
        std::ostream& output) noexcept
{
    ReturnCode_t ret;
    nlohmann::json j;
    if (RETCODE_OK == (ret = json_serialize(traits<DynamicData>::narrow<DynamicDataImpl>(data), j, format)))
    {
        try
        {
            output << j;
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                    "Error encountered while dumping JSON to output stream: " << e.what());
            return RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while performing DynamicData to JSON serialization.");
    }
    return ret;
}

ReturnCode_t json_deserialize(
        const std::string& input,
        const DynamicType::_ref_type& dynamic_type,
        DynamicDataJsonFormat format,
        DynamicData::_ref_type& data) noexcept
{
    nlohmann::json j;
    try
    {
        j = nlohmann::json::parse(input);
        // WARNING: This might result in silent precision loss for numeric values above (u)int64_t and double limits,
        // additional checks should be implemented if willing to cover those cases.
    }
    catch (const std::exception& e) // Exceptions other than nlohmann::json::parse_error can be thrown
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while parsing JSON input: " << e.what());
        return RETCODE_BAD_PARAMETER;
    }

    ReturnCode_t ret;
    if (RETCODE_OK !=
            (ret =
            json_deserialize(j, traits<DynamicType>::narrow<DynamicTypeImpl>(dynamic_type), format, data)))
    {
        EPROSIMA_LOG_ERROR(XTYPES_UTILS,
                "Error encountered while performing JSON to DynamicData deserialization.");
        data.reset(); // Reset data in case of error
    }
    return ret;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
