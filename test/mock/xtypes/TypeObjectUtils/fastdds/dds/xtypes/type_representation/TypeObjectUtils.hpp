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
 * @file TypeObjectUtils.hpp
 * This file contains static functions to help build a TypeObject.
 */

#ifndef FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP
#define FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeObjectUtils
{
public:

    static void type_object_consistency(
            const TypeObject& /*type_object*/)
    {
    }

    static bool is_direct_hash_type_identifier(
            const TypeIdentifier&)
    {
        return false;
    }

    static const NameHash name_hash(
            const std::string&)
    {
        return {};
    }

};

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTUTILS_HPP
