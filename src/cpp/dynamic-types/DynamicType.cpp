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

#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastdds/dds/log/Log.hpp>

#include <dds/core/LengthUnlimited.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicType::DynamicType()
    : descriptor_(nullptr)
    , name_("")
    , kind_(TK_NONE)
    , is_key_defined_(false)
{
}

DynamicType::DynamicType(
        const TypeDescriptor* descriptor)
    : is_key_defined_(false)
{
    descriptor_ = new TypeDescriptor(descriptor);
    try
    {
        name_ = descriptor->get_name();
        kind_ = descriptor->get_kind();
    }
    catch (...)
    {
        name_ = "";
        kind_ = TK_NONE;
    }

    // Alias types use the same members than it's base class.
    if (kind_ == TK_ALIAS)
    {
        for (auto it = descriptor_->get_base_type()->member_by_id_.begin();
                it != descriptor_->get_base_type()->member_by_id_.end(); ++it)
        {
            member_by_name_.insert(std::make_pair(it->second->get_name(), it->second));
        }
    }
}

DynamicType::DynamicType(
        const DynamicTypeBuilder* other)
    : descriptor_(nullptr)
    , name_("")
    , kind_(TK_NONE)
    , is_key_defined_(false)
{
    copy_from_builder(other);
}

DynamicType::~DynamicType()
{
    clear();
}

ReturnCode_t DynamicType::apply_annotation(
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        descriptor_->annotation_.push_back(pNewDescriptor);
        is_key_defined_ = key_annotation();
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicType::apply_annotation(
        const std::string& annotation_name,
        const std::string& key,
        const std::string& value)
{
    AnnotationDescriptor* ann = descriptor_->get_annotation(annotation_name);
    if (ann != nullptr)
    {
        ann->set_value(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->set_type(
            DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive(annotation_name));
        pNewDescriptor->set_value(key, value);
        descriptor_->annotation_.push_back(pNewDescriptor);
        is_key_defined_ = key_annotation();
    }
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DynamicType::apply_annotation_to_member(
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
            logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation to member. The input descriptor isn't consistent.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

ReturnCode_t DynamicType::apply_annotation_to_member(
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
        logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

void DynamicType::clear()
{
    name_ = "";
    kind_ = 0;
    if (descriptor_ != nullptr)
    {
        delete descriptor_;
        descriptor_ = nullptr;
    }

    for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
    {
        delete it->second;
    }
    member_by_id_.clear();
    member_by_name_.clear();
}

ReturnCode_t DynamicType::copy_from_builder(
        const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        clear();

        name_ = other->name_;
        kind_ = other->kind_;
        descriptor_ = new TypeDescriptor(other->descriptor_);

        for (auto it = other->member_by_id_.begin(); it != other->member_by_id_.end(); ++it)
        {
            DynamicTypeMember* newMember = new DynamicTypeMember(it->second);
            newMember->set_parent(this);
            is_key_defined_ = newMember->key_annotation();
            member_by_id_.insert(std::make_pair(newMember->get_id(), newMember));
            member_by_name_.insert(std::make_pair(newMember->get_name(), newMember));
        }

        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicType, invalid input type");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicType::exists_member_by_name(
        const std::string& name) const
{
    if (descriptor_->get_base_type() != nullptr)
    {
        if (descriptor_->get_base_type()->exists_member_by_name(name))
        {
            return true;
        }
    }
    return member_by_name_.find(name) != member_by_name_.end();
}

ReturnCode_t DynamicType::get_descriptor(
        TypeDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(descriptor_);
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting TypeDescriptor, invalid input descriptor");
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }
}

const TypeDescriptor* DynamicType::get_descriptor() const
{
    return descriptor_;
}

TypeDescriptor* DynamicType::get_descriptor()
{
    return descriptor_;
}

bool DynamicType::key_annotation() const
{
    for (auto anIt = descriptor_->annotation_.begin(); anIt != descriptor_->annotation_.end(); ++anIt)
    {
        if ((*anIt)->key_annotation())
        {
            return true;
        }
    }
    return false;
}

bool DynamicType::equals(
        const DynamicType* other) const
{
    if (other != nullptr && descriptor_->annotation_.size() == other->descriptor_->annotation_.size() &&
            member_by_id_.size() == other->member_by_id_.size() &&
            member_by_name_.size() == other->member_by_name_.size())
    {
        // Check the annotation list
        for (auto it = descriptor_->annotation_.begin(),
                it2 = other->descriptor_->annotation_.begin();
                it != descriptor_->annotation_.end(); ++it, ++it2)
        {
            if (!(*it)->equals(*it))
            {
                return false;
            }
        }

        // Check the members by Id
        for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
        {
            auto it2 = other->member_by_id_.find(it->first);
            if (it2 == other->member_by_id_.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        for (auto it = other->member_by_id_.begin(); it != other->member_by_id_.end(); ++it)
        {
            auto it2 = member_by_id_.find(it->first);
            if (it2 == member_by_id_.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        // Check the members by Name
        for (auto it = member_by_name_.begin(); it != member_by_name_.end(); ++it)
        {
            auto it2 = other->member_by_name_.find(it->first);
            if (it2 == other->member_by_name_.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        for (auto it = other->member_by_name_.begin(); it != other->member_by_name_.end(); ++it)
        {
            auto it2 = member_by_name_.find(it->first);
            if (it2 == member_by_name_.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        return true;
    }
    return false;
}

MemberId DynamicType::get_members_count() const
{
    return static_cast<MemberId>(member_by_id_.size());
}

std::string DynamicType::get_name() const
{
    return name_;
}

ReturnCode_t DynamicType::get_member_by_name(
        DynamicTypeMember& member,
        const std::string& name)
{
    auto it = member_by_name_.find(name);
    if (it != member_by_name_.end())
    {
        member = it->second;
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting member by name, member not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

ReturnCode_t DynamicType::get_all_members_by_name(
        std::map<std::string, DynamicTypeMember*>& members)
{
    members = member_by_name_;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DynamicType::get_member(
        DynamicTypeMember& member,
        MemberId id)
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        member = it->second;
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting member, member not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

ReturnCode_t DynamicType::get_all_members(
        std::map<MemberId, DynamicTypeMember*>& members)
{
    members = member_by_id_;
    return ReturnCode_t::RETCODE_OK;
}

uint32_t DynamicType::get_annotation_count()
{
    return static_cast<uint32_t>(descriptor_->annotation_.size());
}

ReturnCode_t DynamicType::get_annotation(
        AnnotationDescriptor& descriptor,
        uint32_t idx)
{
    if (idx < descriptor_->annotation_.size())
    {
        descriptor = *descriptor_->annotation_[idx];
        return ReturnCode_t::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting annotation, annotation not found.");
        return ReturnCode_t::RETCODE_ERROR;
    }
}

DynamicType_ptr DynamicType::get_base_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_base_type();
    }
    return DynamicType_ptr(nullptr);
}

uint32_t DynamicType::get_bounds(
        uint32_t index /*= 0*/) const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_bounds(index);
    }
    return ::dds::core::LENGTH_UNLIMITED;
}

uint32_t DynamicType::get_bounds_size() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_bounds_size();
    }
    return 0;
}

DynamicType_ptr DynamicType::get_discriminator_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_discriminator_type();
    }
    return DynamicType_ptr(nullptr);
}

DynamicType_ptr DynamicType::get_element_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_element_type();
    }
    return DynamicType_ptr(nullptr);
}

DynamicType_ptr DynamicType::get_key_element_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_key_element_type();
    }
    return DynamicType_ptr(nullptr);
}

uint32_t DynamicType::get_total_bounds() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_total_bounds();
    }
    return ::dds::core::LENGTH_UNLIMITED;
}

bool DynamicType::has_children() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_MAP || kind_ == TK_SEQUENCE
           || kind_ == TK_STRUCTURE || kind_ == TK_UNION || kind_ == TK_BITSET;
}

bool DynamicType::is_complex_kind() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_BITMASK || kind_ == TK_ENUM
           || kind_ == TK_MAP || kind_ == TK_SEQUENCE || kind_ == TK_STRUCTURE || kind_ == TK_UNION ||
           kind_ == TK_BITSET;
}

bool DynamicType::is_consistent() const
{
    return descriptor_->is_consistent();
}

bool DynamicType::is_discriminator_type() const
{
    if (kind_ == TK_ALIAS && descriptor_ != nullptr && descriptor_->get_base_type() != nullptr)
    {
        return descriptor_->get_base_type()->is_discriminator_type();
    }
    return kind_ == TK_BOOLEAN || kind_ == TK_BYTE || kind_ == TK_INT16 || kind_ == TK_INT32 ||
           kind_ == TK_INT64 || kind_ == TK_UINT16 || kind_ == TK_UINT32 || kind_ == TK_UINT64 ||
           kind_ == TK_FLOAT32 || kind_ == TK_FLOAT64 || kind_ == TK_FLOAT128 || kind_ == TK_CHAR8 ||
           kind_ == TK_CHAR16 || kind_ == TK_STRING8 || kind_ == TK_STRING16 || kind_ == TK_ENUM || kind_ == TK_BITMASK;
}

void DynamicType::set_name(
        const std::string& name)
{
    if (descriptor_ != nullptr)
    {
        descriptor_->set_name(name);
    }
    name_ = name;
}

size_t DynamicType::get_size() const
{
    switch (kind_)
    {
        case TK_BOOLEAN: case TK_BYTE: case TK_CHAR8: return 1;
        case TK_INT16: case TK_UINT16: case TK_CHAR16:  return 2;
        case TK_INT32: case TK_UINT32: case TK_FLOAT32: return 4;
        case TK_INT64: case TK_UINT64: case TK_FLOAT64: return 8;
        case TK_FLOAT128: return 16;
        case TK_BITMASK: case TK_ENUM:
        {
            size_t bits = descriptor_->get_bounds(0);

            if (bits % 8 == 0)
            {
                return bits / 8;
            }
            else
            {
                return (bits / 8) + 1;
            }
        }
    }
    logError(DYN_TYPES, "Called get_size() within a non primitive type! This is a program's logic error.");
    return 0;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
