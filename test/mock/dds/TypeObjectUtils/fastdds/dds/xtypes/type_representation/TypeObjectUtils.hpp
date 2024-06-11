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
 * @file
 * This file contains static functions to help build a TypeObject.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_

#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class TypeObjectUtils
{
public:

    static void type_object_consistency(
            const TypeObject&)
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

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTUTILS_HPP_
