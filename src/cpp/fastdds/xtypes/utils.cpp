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

#include <bitset>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include <fastdds/dds/xtypes/utils.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>

#include "dynamic_types/DynamicDataImpl.hpp"
#include "dynamic_types/DynamicTypeImpl.hpp"
#include "dynamic_types/DynamicTypeMemberImpl.hpp"
#include "dynamic_types/MemberDescriptorImpl.hpp"
#include "dynamic_types/TypeDescriptorImpl.hpp"

#include "type_conversion/dyn_type_tree.hpp"

#include "utils/collections/TreeNode.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

ReturnCode_t idl_serialize(
        const traits<DynamicType>::ref_type& dynamic_type,
        std::string& output) noexcept
{
    // Create a tree representation of the dynamic type
    utilities::collections::TreeNode<TreeNodeType> parent_type;
    const auto ret = dyn_type_to_tree(dynamic_type, "PARENT", parent_type);

    if (ret != RETCODE_OK)
    {
        return ret;
    }

    // Serialize the tree to IDL
    return dyn_type_tree_to_idl(parent_type, output);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
