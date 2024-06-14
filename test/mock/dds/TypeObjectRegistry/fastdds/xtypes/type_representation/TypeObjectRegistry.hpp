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
 * This file contains the required classes to keep a TypeObject/TypeIdentifier registry.
 */

#ifndef _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
#define _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

class ITypeObjectRegistry
{
};

class TypeObjectRegistry : public ITypeObjectRegistry
{
public:

    ReturnCode_t get_type_object(
            const TypeIdentifier&,
            TypeObject&)
    {
        return fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t register_typeobject_w_dynamic_type(
            const DynamicType::_ref_type&,
            TypeIdentifierPair&)
    {
        return fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_type_information(
            const TypeIdentifierPair&,
            TypeInformation&,
            bool = false)
    {
        return fastdds::dds::RETCODE_OK;
    }

};

} // xtypes
} // dds
} // fastdds
} // eprosima

#endif // _FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION_TYPEOBJECTREGISTRY_HPP_
