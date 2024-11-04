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

#include "TypeAssignability.hpp"

#include <algorithm>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <fastdds/xtypes/type_representation/TypeObjectRegistry.hpp>
#include <rtps/RTPSDomainImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

fastdds::dds::xtypes::TypeObjectRegistry& registry_observer()
{
    return eprosima::fastdds::rtps::RTPSDomainImpl::get_instance()->type_object_registry_observer();
}

bool TypeAssignability::inheritance_type_assignable_from(
        const TypeIdentifier& t1,
        const TypeIdentifier& t2)
{
    if (t1._d() == t2._d())
    {
        return TK_NONE == t1._d() || type_assignable_from(t1, t2);
    }

    return false;
}

bool TypeAssignability::type_assignable_from(
        const TypeIdentifier& t1,
        const TypeIdentifier& t2)
{
    // Preconditions
    if ((EK_MINIMAL != t1._d() && EK_COMPLETE != t1._d()) ||
            (EK_MINIMAL != t2._d() && EK_COMPLETE != t2._d()))
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Precondition fail.");
        return false;
    }

    // Hashes are equivalent.
    if (t1._d() == t2._d() && t1.equivalence_hash() == t2.equivalence_hash())
    {
        return true;
    }

    // Check assignability of minimals.
    TypeIdentifier t1_type_identifier;
    TypeObject t1_minimal_object;
    if (EK_MINIMAL == t1._d())
    {
        t1_type_identifier = t1;
    }
    else
    {
        t1_type_identifier = registry_observer().get_complementary_type_identifier(t1);
    }

    if (RETCODE_OK != registry_observer().get_type_object(t1_type_identifier, t1_minimal_object))
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Cannot retrieve T1 minimal TypeObject from registry.");
    }

    // Check assignability of minimals.
    TypeIdentifier t2_type_identifier;
    TypeObject t2_minimal_object;
    if (EK_MINIMAL == t2._d())
    {
        t2_type_identifier = t2;
    }
    else
    {
        t2_type_identifier = registry_observer().get_complementary_type_identifier(t2);
    }

    if (RETCODE_OK != registry_observer().get_type_object(t2_type_identifier, t2_minimal_object))
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Cannot retrieve T2 minimal TypeObject from registry.");
    }

    return typeobject_assignable_from(t1_minimal_object, t2_minimal_object);
}

bool TypeAssignability::minimal_typeobject_assignable_from(
        const MinimalTypeObject& t1,
        const MinimalTypeObject& t2)
{
    bool ret_value {false};

    // Preconditions
    if (t1._d() != t2._d())
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Precondition fail.");
        return ret_value;
    }

    switch (t1._d())
    {
        case TK_ALIAS:
            ret_value = t1.alias_type() == t2.alias_type();
            break;
        case TK_ANNOTATION:
            ret_value = t1.annotation_type() == t2.annotation_type();
            break;
        case TK_STRUCTURE:
            ret_value = struct_type_assignable_from(t1.struct_type(), t2.struct_type());
            break;
        case TK_UNION:
            ret_value = true; //t1.union_type() == t2.union_type();
            break;
        case TK_BITSET:
            ret_value = t1.bitset_type() == t2.bitset_type();
            break;
        case TK_SEQUENCE:
            ret_value = t1.sequence_type() == t2.sequence_type();
            break;
        case TK_ARRAY:
            ret_value = t1.array_type() == t2.array_type();
            break;
        case TK_MAP:
            ret_value = t1.map_type() == t2.map_type();
            break;
        case TK_ENUM:
            ret_value = true; //t1.enumerated_type() == t2.enumerated_type();
            break;
        case TK_BITMASK:
            ret_value = t1.bitmask_type() == t2.bitmask_type();
            break;
    }

    return ret_value;
}

bool TypeAssignability::struct_type_assignable_from(
        const MinimalStructType& t1,
        const MinimalStructType& t2)
{
    if (t1.struct_flags() == t2.struct_flags() && // TODO (richiware) Check what to do with NESTED and AUTOHASH
            inheritance_type_assignable_from(t1.header().base_type(), t2.header().base_type()))
    {
        size_t pos {0};
        std::list<const MinimalStructMember*> t1_member_list {t1.member_seq().size()};
        std::list<const MinimalStructMember*> t2_member_list {t2.member_seq().size()};
        std::generate(t1_member_list.begin(), t1_member_list.end(), [&pos, &t1]()
                {
                    return &t1.member_seq().at(pos++);
                });
        pos = 0;
        std::generate(t2_member_list.begin(), t2_member_list.end(), [&pos, &t2]()
                {
                    return &t2.member_seq().at(pos++);
                });

        /*
           for (auto& t1_member : t1.member_seq())
           {
           auto& t2_member = std::find_if(t2.member_seq().begin(), t2.member_seq().end(), [&t1_member](const MinimalStructMember& x)
                                      {
                                      return x.common().member_id() ==
                                      };
           }
         */
    }

    return true; //false;
}

bool TypeAssignability::typeobject_assignable_from(
        const TypeObject& t1,
        const TypeObject& t2)
{
    // Preconditions
    if (EK_MINIMAL != t1._d() || EK_MINIMAL != t2._d())
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Precondition fail.");
        return false;
    }

    return minimal_typeobject_assignable_from(t1.minimal(), t2.minimal());
}

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
