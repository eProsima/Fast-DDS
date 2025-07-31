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

    bool ret_value {true};

    // Check assignability of minimals.
    MinimalTypeObject t1_minimal_object;
    ret_value &= get_minimal_type_object(t1, t1_minimal_object);

    // Check assignability of minimals.
    MinimalTypeObject t2_minimal_object;
    ret_value &= get_minimal_type_object(t2, t2_minimal_object);


    return ret_value && minimal_typeobject_assignable_from(t1_minimal_object, t2_minimal_object);
}

bool TypeAssignability::type_assignable_from_(
        const TypeIdentifier& t1,
        const TypeIdentifier& t2)
{
    bool ret_value {false};
    MinimalTypeObject aux_minimal_object;

    switch (t1._d())
    {
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT8:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_CHAR8:
        case TK_CHAR16:
            ret_value = t1._d() == t2._d();
            break;
        case TK_UINT8:
            ret_value = TK_UINT8 == t2._d() ||
                    (EK_MINIMAL == t2._d() && get_minimal_type_object(t2, aux_minimal_object) &&
                    TK_BITMASK == aux_minimal_object._d() &&
                    1 <= aux_minimal_object.bitmask_type().header().common().bit_bound() &&
                    8 >= aux_minimal_object.bitmask_type().header().common().bit_bound());
            break;
        case TK_UINT16:
            ret_value = TK_UINT16 == t2._d() ||
                    (EK_MINIMAL == t2._d() && get_minimal_type_object(t2, aux_minimal_object) &&
                    TK_BITMASK == aux_minimal_object._d() &&
                    9 <= aux_minimal_object.bitmask_type().header().common().bit_bound() &&
                    16 >= aux_minimal_object.bitmask_type().header().common().bit_bound());
            break;
        case TK_UINT32:
            ret_value = TK_UINT32 == t2._d() ||
                    (EK_MINIMAL == t2._d() && get_minimal_type_object(t2, aux_minimal_object) &&
                    TK_BITMASK == aux_minimal_object._d() &&
                    17 <= aux_minimal_object.bitmask_type().header().common().bit_bound() &&
                    32 >= aux_minimal_object.bitmask_type().header().common().bit_bound());
            break;
        case TK_UINT64:
            ret_value = TK_UINT64 == t2._d() ||
                    (EK_MINIMAL == t2._d() && get_minimal_type_object(t2, aux_minimal_object) &&
                    TK_BITMASK == aux_minimal_object._d() &&
                    33 <= aux_minimal_object.bitmask_type().header().common().bit_bound() &&
                    64 >= aux_minimal_object.bitmask_type().header().common().bit_bound());
            break;
        case TI_STRING8_SMALL:
            ret_value = (t1._d() == t2._d()) && t1.string_sdefn() == t2.string_sdefn();
            break;
        case TI_STRING8_LARGE:
            ret_value = (t1._d() == t2._d()) && t1.string_ldefn() == t2.string_ldefn();
            break;
        case EK_MINIMAL:
        case EK_COMPLETE:
            ret_value = type_assignable_from(t1, t2);
    }

    return ret_value;
}

bool TypeAssignability::minimal_typeobject_assignable_from(
        const MinimalTypeObject& t1,
        const MinimalTypeObject& t2)
{
    bool ret_value {false};

    switch (t1._d())
    {
        case TK_ALIAS:
            ret_value = t1.alias_type() == t2.alias_type();
            break;
        case TK_ANNOTATION:
            ret_value = t1.annotation_type() == t2.annotation_type();
            break;
        case TK_ARRAY:
            ret_value = t1.array_type() == t2.array_type();
            break;
        case TK_BITMASK:
            ret_value = t1.bitmask_type() == t2.bitmask_type();
            break;
        case TK_BITSET:
            ret_value = t1.bitset_type() == t2.bitset_type();
            break;
        case TK_ENUM:
            ret_value = t1.enumerated_type() == t2.enumerated_type();
            break;
        case TK_MAP:
            ret_value = t1.map_type() == t2.map_type();
            break;
        case TK_SEQUENCE:
            ret_value = t1.sequence_type() == t2.sequence_type();
            break;
        case TK_STRUCTURE:
            ret_value = (t1._d() == t2._d()) && struct_type_assignable_from(t1.struct_type(), t2.struct_type());
            break;
        case TK_UNION:
            ret_value = t1.union_type() == t2.union_type();
            break;
        default:
            ret_value = false;
            break;
    }

    return ret_value;
}

bool TypeAssignability::struct_type_assignable_from(
        const MinimalStructType& t1,
        const MinimalStructType& t2)
{
    StructTypeFlag t1_flags = t1.struct_flags() &
            (TypeFlagBits::IS_FINAL | TypeFlagBits::IS_APPENDABLE | TypeFlagBits::IS_MUTABLE);
    StructTypeFlag t2_flags = t2.struct_flags() &
            (TypeFlagBits::IS_FINAL | TypeFlagBits::IS_APPENDABLE | TypeFlagBits::IS_MUTABLE);

    // X-Types 1.3: T1 and T2 have the same extensibility kind.
    bool ret_value {t1_flags == t2_flags};

    if (ret_value)
    {
        bool is_final = t1_flags & TypeFlagBits::IS_FINAL;
        bool is_final_or_appendable = t1_flags & (TypeFlagBits::IS_FINAL | TypeFlagBits::IS_APPENDABLE);

        std::list<const MinimalStructMember*> t1_member_list;
        generate_all_struct_members(t1_member_list, t1);
        std::list<const MinimalStructMember*> t2_member_list;
        generate_all_struct_members(t2_member_list, t2);
        size_t t2_member_list_size {t2_member_list.size()};

        // X-Types 1.3: AND if T1 is final, then they meet the same condition as for T1 being appendable and in
        // addition T1 and T2 have the same set of member IDs.
        ret_value &= !is_final || (t1_member_list.size() == t2_member_list.size());

        for (auto t1_member {t1_member_list.begin()}; t1_member != t1_member_list.end() && ret_value;)
        {
            std::list<const MinimalStructMember*>::const_iterator t2_member;

            if (is_final_or_appendable)
            {
                t2_member = t2_member_list.begin();
            }
            else
            {
                t2_member = std::find_if(t2_member_list.begin(), t2_member_list.end(),
                                [&t1_member](const MinimalStructMember* x)
                                {
                                    return x->common().member_id() == (*t1_member)->common().member_id();
                                });
            }

            // X-Types 1.3: Any members in T1 and T2 that have the same name also have the same ID and any members with
            // the same ID also have the same name.
            if (t2_member != t2_member_list.end())
            {
                ret_value &= (*t1_member)->common().member_id() == (*t2_member)->common().member_id();
                ret_value &= (*t1_member)->detail().name_hash() == (*t2_member)->detail().name_hash();
                ret_value &=
                        TypeAssignability::type_assignable_from_(
                    (*t1_member)->common().member_type_id(),
                    (*t2_member)->common().member_type_id());

                // X-Types 1.3: If T1 and T2 have the same member with the same ID, then the member in T1 is assignable
                // from the member in T2.
                //ret_value &= TypeAssignability::type_assignable_from( (*t1_member)->common().type_identifier(), (*t2_member)->common().type_identifier());

                t2_member_list.erase(t2_member);
            }
            else
            {
                if (!is_final_or_appendable)
                {
                    // X-Types 1.3: Any members in T1 and T2 that have the same name also have the same ID and any members with
                    // the same ID also have the same name.
                    ret_value &= t2_member_list.end() == std::find_if(t2_member_list.begin(), t2_member_list.end(),
                                    [&t1_member](const MinimalStructMember* x)
                                    {
                                        return x->detail().name_hash() == (*t1_member)->detail().name_hash();
                                    });
                }
                else if (is_final)
                {
                    // X-Types 1.3: AND if T1 is appendable, then members with the same member_index have the same
                    // member ID, the same setting for the ‘optional’ attribute and the T1 member type is strongly
                    // assignable from the T2 member type.
                    ret_value = false;
                }
            }

            t1_member = t1_member_list.erase(t1_member);
        }

        // X-Types 1.3: There is at least one member “m1” of T1 and one corresponding member “m2” of T2 such that
        // m1.id == m2.id
        ret_value &= t2_member_list.size() < t2_member_list_size;
    }

    return ret_value;
}

void TypeAssignability::generate_all_struct_members(
        std::list<const MinimalStructMember*>& list,
        const MinimalStructType& type)
{
    if (TK_NONE != type.header().base_type()._d())
    {
        MinimalTypeObject base_type_object;
        get_minimal_type_object(type.header().base_type(), base_type_object);

        if (TK_STRUCTURE == base_type_object._d())
        {
            generate_all_struct_members(list, base_type_object.struct_type());
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Base type is not a structure.");
            return;
        }
    }


    for (size_t pos {0}; pos < type.member_seq().size(); ++pos)
    {
        list.push_back(&type.member_seq().at(pos));
    }
}

bool TypeAssignability::get_minimal_type_object(
        TypeIdentifier type_id,
        MinimalTypeObject& minimal_object)
{
    TypeIdentifier type_identifier;

    if (EK_MINIMAL == type_id._d())
    {
        type_identifier = type_id;
    }
    else if (EK_COMPLETE == type_id._d())
    {
        type_identifier = registry_observer().get_complementary_type_identifier(type_id);
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "TypeIdentifier is not minimal or complete.");
        return false;
    }

    bool ret_value {false};
    TypeObject type_object;

    if (RETCODE_OK == registry_observer().get_type_object(type_identifier, type_object))
    {
        if (EK_MINIMAL == type_object._d())
        {
            minimal_object = type_object.minimal();
            ret_value = true;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "TypeObject is not minimal.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Cannot retrieve T1 minimal TypeObject from registry.");
    }

    return ret_value;
}

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima
