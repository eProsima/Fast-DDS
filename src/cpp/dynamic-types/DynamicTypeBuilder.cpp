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
    , max_index_(0)
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

DynamicTypeBuilder::~DynamicTypeBuilder()
{
    TypeDescriptor::clean();

    for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
    {
        delete it->second;
    }
    member_by_id_.clear();
    member_by_name_.clear();
}

ReturnCode_t DynamicTypeBuilder::add_empty_member(
        uint32_t index,
        const std::string& name)
{
    MemberDescriptor descriptor(index, name);
    if (get_kind() == TK_BITMASK)
    {
        if (index >= get_bounds(0))
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, out of bounds.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        descriptor.annotation_set_position(static_cast<uint16_t>(descriptor.get_index()));
    }
    return add_member(&descriptor);
}

ReturnCode_t DynamicTypeBuilder::add_member(
        const MemberDescriptor* descriptor)
{
    if (descriptor != nullptr && descriptor->is_consistent(get_kind()))
    {
        if (get_kind() == TK_ANNOTATION || get_kind() == TK_BITMASK
                || get_kind() == TK_ENUM || get_kind() == TK_STRUCTURE
                || get_kind() == TK_UNION || get_kind() == TK_BITSET)
        {
            if (!exists_member_by_name(descriptor->get_name()) ||
                    (kind_ == TK_BITSET && descriptor->get_name().empty())) // Bitsets allow multiple empty members.
            {
                if (check_union_configuration(descriptor))
                {
                    DynamicTypeMember* newMember = new DynamicTypeMember(descriptor, current_member_id_);

                    // If the index of the new member is bigger than the current maximum, put it at the end.
                    if (newMember->get_index() > max_index_)
                    {
                        newMember->set_index(max_index_++);
                    }
                    else
                    {
                        // Move every member bigger than the current index to the right.
                        for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
                        {
                            if (it->second->get_index() >= newMember->get_index())
                            {
                                it->second->set_index(it->second->get_index() + 1);
                            }
                        }
                    }

                    if (!descriptor->get_name().empty()) // Don't store empty bitset members.
                    {
                        member_by_id_.insert(std::make_pair(current_member_id_, newMember));
                        member_by_name_.insert(std::make_pair(newMember->get_name(), newMember));
                    }
                    else
                    {
                        delete newMember;
                    }
                    ++current_member_id_;
                    return ReturnCode_t::RETCODE_OK;
                }
                else
                {
                    EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, invalid union parameters.");
                    return ReturnCode_t::RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, there is other member with the same name.");
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
        if (descriptor == nullptr)
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, Invalid input descriptor.");
        }
        else
        {
            EPROSIMA_LOG_WARNING(DYN_TYPES, "Error adding member, The input descriptor isn't consistent.");
        }
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
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

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicTypeBuilder* type)
{
    if (type != nullptr)
    {
        MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::get_instance()->create_type(type));
        return add_member(&descriptor);
    }
    else
    {
        MemberDescriptor descriptor(id, name, DynamicType_ptr(nullptr));
        return add_member(&descriptor);
    }
}

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicTypeBuilder* type,
        const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::get_instance()->create_type(type), defaultValue);
    return add_member(&descriptor);
}

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicTypeBuilder* type,
        const std::string& defaultValue,
        const std::vector<uint64_t>& unionLabels,
        bool isDefaultLabel)
{
    MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::get_instance()->create_type(type),
            defaultValue, unionLabels, isDefaultLabel);
    return add_member(&descriptor);
}

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type)
{
    MemberDescriptor descriptor(id, name, type);
    return add_member(&descriptor);
}

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type,
        const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, type, defaultValue);
    return add_member(&descriptor);
}

ReturnCode_t DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type_,
        const std::string& defaultValue,
        const std::vector<uint64_t>& unionLabels,
        bool isDefaultLabel)
{
    MemberDescriptor descriptor(id, name, type_, defaultValue, unionLabels, isDefaultLabel);
    return add_member(&descriptor);
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
        const MemberDescriptor* descriptor)
{
    if (get_kind() == TK_UNION)
    {
        if (!descriptor->is_default_union_value() && descriptor->get_union_labels().size() == 0)
        {
            return false;
        }
        for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
        {
            // Check that there isn't any member as default label and that there isn't any member with the same case.
            if ((descriptor->is_default_union_value() && it->second->is_default_union_value()) ||
                    !descriptor->check_union_labels(it->second->get_union_labels()))
            {
                return false;
            }
        }
    }
    return true;
}

void DynamicTypeBuilder::clear()
{
    for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
    {
        delete it->second;
    }
    member_by_id_.clear();
    member_by_name_.clear();
    current_member_id_ = 0;
}

bool DynamicTypeBuilder::exists_member_by_name(
        const std::string& name) const
{
    if (get_base_type() != nullptr)
    {
        if (get_base_type()->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_name_.find(name) != member_by_name_.end();
}

bool DynamicTypeBuilder::is_discriminator_type() const
{
    if (kind_ == TK_ALIAS && get_base_type() != nullptr)
    {
        return get_base_type()->is_discriminator_type();
    }
    return kind_ == TK_BOOLEAN || kind_ == TK_BYTE || kind_ == TK_INT16 || kind_ == TK_INT32 ||
           kind_ == TK_INT64 || kind_ == TK_UINT16 || kind_ == TK_UINT32 || kind_ == TK_UINT64 ||
           kind_ == TK_FLOAT32 || kind_ == TK_FLOAT64 || kind_ == TK_FLOAT128 || kind_ == TK_CHAR8 ||
           kind_ == TK_CHAR16 || kind_ == TK_STRING8 || kind_ == TK_STRING16 || kind_ == TK_ENUM || kind_ == TK_BITMASK;
}

void DynamicTypeBuilder::refresh_member_ids()
{
    if ((get_kind() == TK_STRUCTURE || get_kind() == TK_BITSET) &&
            get_base_type() != nullptr)
    {
        current_member_id_ = get_base_type()->get_members_count();
    }
}

ReturnCode_t DynamicTypeBuilder::apply_annotation_to_member(
        MemberId id,
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        auto it = member_by_id_.find(id);
        if (it != member_by_id_.end())
        {
            it->second->apply_annotation(descriptor);
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation to member. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicTypeBuilder::apply_annotation_to_member(
        MemberId id,
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        it->second->apply_annotation(annotation_name, key, value);
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicTypeBuilder::apply_annotation(
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        annotation_.push_back(descriptor);
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
    AnnotationDescriptor* ann = get_annotation(annotation_name);
    if (ann != nullptr)
    {
        ann->set_value(key, value);
    }
    else
    {
        AnnotationDescriptor new_descriptor;
        new_descriptor.set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(annotation_name));
        new_descriptor.set_value(key, value);
        annotation_.push_back(new_descriptor);
        is_key_defined_ = key_annotation();
    }
    return ReturnCode_t::RETCODE_OK;
}

// Annotation setters
void DynamicTypeBuilder::annotation_set_extensibility(
        const std::string& extensibility)
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_EXTENSIBILITY_ID));
        apply_annotation(descriptor);
        ann = get_annotation(ANNOTATION_EXTENSIBILITY_ID);
    }
    ann->set_value("value", extensibility);
}

void DynamicTypeBuilder::annotation_set_mutable()
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_MUTABLE_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_MUTABLE_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_MUTABLE_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_final()
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_FINAL_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_FINAL_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_FINAL_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_appendable()
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_APPENDABLE_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_APPENDABLE_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_APPENDABLE_ID);
    }
    ann->set_value("value", CONST_TRUE);
}

void DynamicTypeBuilder::annotation_set_nested(
        bool nested)
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_NESTED_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_NESTED_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_NESTED_ID);
    }
    ann->set_value("value", nested ? CONST_TRUE : CONST_FALSE);
}

void DynamicTypeBuilder::annotation_set_key(
        bool key)
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_KEY_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_KEY_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_KEY_ID);
    }
    ann->set_value("value", key ? CONST_TRUE : CONST_FALSE);
}

void DynamicTypeBuilder::annotation_set_bit_bound(
        uint16_t bit_bound)
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_BIT_BOUND_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_BIT_BOUND_ID));
        apply_annotation(*ann);
        ann = get_annotation(ANNOTATION_BIT_BOUND_ID);
    }
    ann->set_value("value", std::to_string(bit_bound));
}

void DynamicTypeBuilder::annotation_set_non_serialized(
        bool non_serialized)
{
    const AnnotationDescriptor* ann = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    if (ann == nullptr)
    {
        AnnotationDescriptor descriptor;
        descriptor.set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(ANNOTATION_NON_SERIALIZED_ID));
        apply_annotation(descriptor);
        ann = get_annotation(ANNOTATION_NON_SERIALIZED_ID);
    }
    ann->set_value("value", non_serialized ? CONST_TRUE : CONST_FALSE);
}

bool DynamicTypeBuilder::equals(
        const DynamicType& other) const
{
    return get_type_descriptor() == other.get_type_descriptor();
}
