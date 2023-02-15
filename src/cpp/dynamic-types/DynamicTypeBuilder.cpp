// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastdds/dds/log/Log.hpp>

#include <tuple>

using namespace eprosima::fastrtps::types;

DynamicTypeBuilder::DynamicTypeBuilder(
        use_the_create_method)
{
}

DynamicTypeBuilder::DynamicTypeBuilder(
        use_the_create_method,
        const DynamicTypeBuilder* builder)
{
    assert(builder);
    *this = *builder;
}

DynamicTypeBuilder::DynamicTypeBuilder(
        use_the_create_method,
        const TypeDescriptor* descriptor)
    : current_member_id_(0)
{
    static_assert(false);
//    descriptor_ = new TypeDescriptor(descriptor);
//    try
//    {
//        name_ = descriptor->get_name();
//        kind_ = descriptor->get_kind();
//    }
//    catch (...)
//    {
//        name_ = "";
//        kind_ = TK_NONE;
//    }
//
//    // Alias types use the same members than it's base class.
//    if (kind_ == TK_ALIAS)
//    {
//        for (auto it = descriptor_->get_base_type()->member_by_id_.begin();
//                it != descriptor_->get_base_type()->member_by_id_.end(); ++it)
//        {
//            member_by_name_.insert(std::make_pair(it->second->get_name(), it->second));
//        }
//    }
//
//    refresh_member_ids();
}

member_iterator DynamicTypeBuilder::add_empty_member(
        uint32_t index,
        const std::string& name)
{
    // insert the new member
    member_iterator it;
    if( index >=  members_.size() )
    {
        // at the end
        index = members_.size();
        it = members_.emplace_back(index, name);
    }
    else
    {
        // move all the others
        it = members_.begin();
        std::advance(it, index);
        it = members_.emplace(it, index, name);
        // rename the others
        for(auto i = index; it != members_.end(); ++it)
        {
            it->set_index(++i);
            assert(it->get_index() == i);
        }
    }

    if (get_kind() == TK_BITMASK && index >= get_bounds(0))
    {
        EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, out of bounds.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;

        it->annotation_set_position(static_cast<uint16_t>(index));
    }

    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DynamicTypeBuilder::add_member(
        const MemberDescriptor& descriptor)
{
    if (descriptor.is_consistent(get_kind()))
    {
        if (get_kind() == TK_ANNOTATION || get_kind() == TK_BITMASK
                || get_kind() == TK_ENUM || get_kind() == TK_STRUCTURE
                || get_kind() == TK_UNION || get_kind() == TK_BITSET)
        {
            auto member_name = descriptor.get_name();

            // Bitsets allow multiple empty members.
            if( kind_ != TK_BITSET && descriptor.get_name().empty())
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding member, missing proper name.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            if(!member_name.empty() && exists_member_by_name(member_name) ||
            )
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding member, there is other member with the same name.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            auto member_id = descriptor.get_id();
            if (member_id != MEMBER_ID_INVALID && exists_member_by_id(member_id))
            {
                EPROSIMA_LOG_ERROR(DYN_TYPES, "Error adding member, there is other member with the same id.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }

            if (check_union_configuration(descriptor))
            {
                auto it = add_empty_member(descriptor.get_index(), member_name);

                DynamicTypeMember& newMember = *it;
                // Copy all elements but keep the index
                auto member_index = newMember.get_index();
                newMember = descriptor;
                newMember.set_index(member_index);

                if(member_id == MEMBER_ID_INVALID)
                {
                    // assing a new one
                    while(exists_member_by_id(current_member_id_))
                    {
                        member_id = ++current_member_id_;
                    }

                    newMember.set_id(member_id);
                }

                // update the indexes collections
                if (!member_name.empty()) // Don't store empty bitset members.
                {
                    member_by_id_.insert(std::make_pair(member_id, &newMember));
                    member_by_name_.insert(std::make_pair(member_name, &newMember));
                }

                // advance
                ++current_member_id_;
            }
            else
            {
                EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, invalid union parameters.");
                return ReturnCode_t::RETCODE_BAD_PARAMETER;
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, the current type " << get_kind()
                                                                                     << " doesn't support members.");
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    return ReturnCode_t::RETCODE_OK;
}

RTPS_DllAPI MemberId DynamicTypeBuilder::get_member_id_by_name(
        const std::string& name) const
{
    auto it = member_by_name_.find(name);
    if (it != member_by_name_.end())
    {
        return it->second->get_id();
    }
    return MEMBER_ID_INVALID;
}

DynamicType_ptr DynamicTypeBuilder::build() const
{
    if (is_consistent())
    {
        return DynamicTypeBuilderFactory::get_instance()->create_type(this);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type. The current descriptor isn't consistent.");
        return nullptr;
    }
}

bool DynamicTypeBuilder::check_union_configuration(
        const MemberDescriptor& descriptor)
{
    if (get_kind() == TK_UNION)
    {
        bool default_union_value = descriptor.is_default_union_value();
        if (!default_union_value && descriptor.get_union_labels().size() == 0)
        {
            return false;
        }
        for (auto& m : members_ )
        {
            // Check that there isn't any member as default label and that there isn't any member with the same case.
            if (default_union_value && m.is_default_union_value() ||
                    !descriptor.check_union_labels(m.get_union_labels()))
            {
                return false;
            }
        }
    }
    return true;
}

void DynamicTypeBuilder::clear()
{
    TypeDescriptor::clean();
    current_member_id_ = 0;
}

bool DynamicTypeBuilder::exists_member_by_name(
        const std::string& name) const
{
    auto base = get_base_type();
    if (base)
    {
        if (base->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_name_.find(name) != member_by_name_.end();
}

bool DynamicTypeBuilder::exists_member_by_id(
        MemberId id) const
{
    auto base = get_base_type();
    if (base)
    {
        if (base->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_id_.find(id) != member_by_id_.end();
}

bool DynamicTypeBuilder::is_discriminator_type() const
{
    auto base = get_base_type();
    auto kind = get_kind();

    if (kind == TK_ALIAS && base)
    {
        return base->is_discriminator_type();
    }
    return kind == TK_BOOLEAN || kind == TK_BYTE || kind == TK_INT16 || kind == TK_INT32 ||
           kind == TK_INT64 || kind == TK_UINT16 || kind == TK_UINT32 || kind == TK_UINT64 ||
           kind == TK_FLOAT32 || kind == TK_FLOAT64 || kind == TK_FLOAT128 || kind == TK_CHAR8 ||
           kind == TK_CHAR16 || kind == TK_STRING8 || kind == TK_STRING16 || kind == TK_ENUM || kind == TK_BITMASK;
}

void DynamicTypeBuilder::refresh_member_ids()
{
    auto base = get_base_type();
    auto kind = get_kind();

    if ((kind == TK_STRUCTURE || kind == TK_BITSET) && base)
    {
        current_member_id_ = base->get_members_count();
    }
}

ReturnCode_t DynamicTypeBuilder::apply_annotation(
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        bool inserted = false;

        std::tie(std::ignore, inserted) = annotation_.insert(descriptor);

        if(!inserted)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation: it was already there.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }

        is_key_defined_ = key_annotation();

        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicTypeBuilder::apply_annotation(
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(annotation_name);
    if (found)
    {
        ann->set_value(key, value);
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        AnnotationDescriptor new_descriptor;
        new_descriptor.set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(annotation_name));
        new_descriptor.set_value(key, value);
        return apply_annotation(new_descriptor);
    }
}

// Annotation setters

/* Ancillary method for setters
 * @param id annotation name
 * @param C functor that checks if the annotation should be modified: bool(const AnnotationDescriptor&)
 * @param M functor that modifies the annotation if present: void(AnnotationDescriptor&)
 */
template<typename C, typename M>
void DynamicTypeBuilder::annotation_set( const std::string& id, C& c, M& m)
{
    annotation_iterator it;
    bool found;

    std::tie(it, found) = get_annotation(id);
    if(!found)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(id));
        m(descriptor);
        apply_annotation(descriptor);
    }
    else if(c(*it))
    {
        // Reinsert because order may be modified
        AnnotationDescriptor descriptor(std::move(*it));
        it = annotation_.erase(it);
        m(descriptor);
        annotation_.insert(it, std::move(descriptor));
    }

    std::tie(it, found) = get_annotation(id);
    assert(found);
}

//! Specialization of the above template to simple values
void DynamicTypeBuilder::annotation_set(const std::string& id, std::string& new_val);
{
    annotation_set(
            id,
            [&new_val](const AnnotationDescriptor& d) -> bool
            {
               std::string val;
               d.get_value(val, "value");
               return new_val != val;
            },
            [&new_val](AnnotationDescriptor& d)
            {
                d.set_value("value", new_val);
            });
}

void DynamicTypeBuilder::annotation_set_extensibility(
        const std::string& extensibility)
{
    annotation_set(ANNOTATION_EXTENSIBILITY_ID, extensibility);
}

void DynamicTypeBuilder::annotation_set_mutable()
{
    annotation_set(ANNOTATION_MUTABLE_ID, CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_final()
{
    annotation_set(ANNOTATION_FINAL_ID, CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_appendable()
{
    annotation_set(ANNOTATION_APPENDABLE_ID, CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_nested(
        bool nested)
{
    annotation_set(ANNOTATION_NESTED_ID, CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_key(
        bool key)
{
    annotation_set(ANNOTATION_KEY_ID, key ? CONST_TRUE : CONST_FALSE);
}

void DynamicTypeBuilder::annotation_set_bit_bound(
        uint16_t bit_bound)
{
    annotation_set(ANNOTATION_BIT_BOUND_ID, std::to_string(bit_bound));
}

void DynamicTypeBuilder::annotation_set_non_serialized(
        bool non_serialized)
{
    annotation_set(ANNOTATION_NON_SERIALIZED_ID, non_serialized ? CONST_TRUE : CONST_FALSE);
}

bool DynamicTypeBuilder::equals(
        const DynamicType& other) const
{
    return get_type_descriptor() == other.get_type_descriptor();
}
