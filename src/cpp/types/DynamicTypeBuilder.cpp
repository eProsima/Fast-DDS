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
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilder::DynamicTypeBuilder()
    : descriptor_(nullptr)
    , current_member_id_(0)
    , max_index_(0)
{
}

DynamicTypeBuilder::DynamicTypeBuilder(const DynamicTypeBuilder* builder)
    : current_member_id_(0)
    , max_index_(0)
{
    copy_from_builder(builder);
}

DynamicTypeBuilder::DynamicTypeBuilder(const TypeDescriptor* descriptor)
    : current_member_id_(0)
    , max_index_(0)
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

    refresh_member_ids();
}

DynamicTypeBuilder::~DynamicTypeBuilder()
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

ResponseCode DynamicTypeBuilder::add_empty_member(
        uint32_t index,
        const std::string& name)
{
    MemberDescriptor descriptor(index, name);
    return add_member(&descriptor);
}

ResponseCode DynamicTypeBuilder::add_member(const MemberDescriptor* descriptor)
{
    if (descriptor_ != nullptr && descriptor != nullptr && descriptor->is_consistent(descriptor_->get_kind()))
    {
        if (descriptor_->get_kind() == TK_ANNOTATION || descriptor_->get_kind() == TK_BITMASK
            || descriptor_->get_kind() == TK_ENUM || descriptor_->get_kind() == TK_STRUCTURE
            || descriptor_->get_kind() == TK_UNION)
        {
            if (!exists_member_by_name(descriptor->get_name()))
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

                    member_by_id_.insert(std::make_pair(current_member_id_, newMember));
                    member_by_name_.insert(std::make_pair(newMember->get_name(), newMember));
                    ++current_member_id_;
                    return ResponseCode::RETCODE_OK;
                }
                else
                {
                    logWarning(DYN_TYPES, "Error adding member, invalid union parameters.");
                    return ResponseCode::RETCODE_BAD_PARAMETER;
                }
            }
            else
            {
                logWarning(DYN_TYPES, "Error adding member, there is other member with the same name.");
                return ResponseCode::RETCODE_BAD_PARAMETER;
            }
        }
        else
        {
            logWarning(DYN_TYPES, "Error adding member, the current type " << descriptor_->get_kind()
                << " doesn't support members.");
            return ResponseCode::RETCODE_PRECONDITION_NOT_MET;
        }
    }
    else
    {
        if (descriptor == nullptr)
        {
            logWarning(DYN_TYPES, "Error adding member, Invalid input descriptor.");
        }
        else
        {
            logWarning(DYN_TYPES, "Error adding member, The input descriptor isn't consistent.");
        }
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicTypeBuilder::add_member(
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
        MemberDescriptor descriptor(id, name, nullptr);
        return add_member(&descriptor);
    }
}

ResponseCode DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicTypeBuilder* type,
        const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, DynamicTypeBuilderFactory::get_instance()->create_type(type), defaultValue);
    return add_member(&descriptor);
}

ResponseCode DynamicTypeBuilder::add_member(
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

ResponseCode DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type)
{
    MemberDescriptor descriptor(id, name, type);
    return add_member(&descriptor);
}

ResponseCode DynamicTypeBuilder::add_member(
        MemberId id,
        const std::string& name,
        DynamicType_ptr type,
        const std::string& defaultValue)
{
    MemberDescriptor descriptor(id, name, type, defaultValue);
    return add_member(&descriptor);
}

ResponseCode DynamicTypeBuilder::add_member(
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

ResponseCode DynamicTypeBuilder::apply_annotation(AnnotationDescriptor& descriptor)
{
    return descriptor_->apply_annotation(descriptor);
}

ResponseCode DynamicTypeBuilder::apply_annotation(
        const std::string& key,
        const std::string& value)
{
    return descriptor_->apply_annotation(key, value);
}

ResponseCode DynamicTypeBuilder::apply_annotation_to_member(
        MemberId id,
        AnnotationDescriptor& descriptor)
{
    return _apply_annotation_to_member(id, descriptor);
}

ResponseCode DynamicTypeBuilder::apply_annotation_to_member(
        MemberId id,
        const std::string& key,
        const std::string& value)
{
    return _apply_annotation_to_member(id, key, value);
}

DynamicType_ptr DynamicTypeBuilder::build()
{
    if (descriptor_->is_consistent())
    {
        return DynamicTypeBuilderFactory::get_instance()->create_type(this);
    }
    else
    {
        logError(DYN_TYPES, "Error building type. The current descriptor isn't consistent.");
        return nullptr;
    }
}

bool DynamicTypeBuilder::check_union_configuration(const MemberDescriptor* descriptor)
{
    if (descriptor_->get_kind() == TK_UNION)
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

ResponseCode DynamicTypeBuilder::copy_from(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        clear();

        ResponseCode res = copy_from_builder(other);
        if (res == ResponseCode::RETCODE_OK)
        {
            current_member_id_ = other->current_member_id_;
        }
        return res;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicTypeBuilder. Invalid input parameter.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}


ResponseCode DynamicTypeBuilder::copy_from_builder(const DynamicTypeBuilder* other)
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
            member_by_id_.insert(std::make_pair(newMember->get_id(), newMember));
            member_by_name_.insert(std::make_pair(newMember->get_name(), newMember));
        }

        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicType, invalid input type");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

void DynamicTypeBuilder::clear()
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
    current_member_id_ = 0;
}

bool DynamicTypeBuilder::exists_member_by_name(const std::string& name) const
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

ResponseCode DynamicTypeBuilder::get_all_members(std::map<MemberId, DynamicTypeMember*>& members)
{
    members = member_by_id_;
    return ResponseCode::RETCODE_OK;
}

std::string DynamicTypeBuilder::get_name() const
{
    return name_;
}

bool DynamicTypeBuilder::is_consistent() const
{
    return descriptor_->is_consistent();
}

bool DynamicTypeBuilder::is_discriminator_type() const
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

void DynamicTypeBuilder::refresh_member_ids()
{
    if (descriptor_->get_kind() == TK_STRUCTURE && descriptor_->get_base_type() != nullptr)
    {
        current_member_id_ = descriptor_->get_base_type()->get_members_count();
    }
}

ResponseCode DynamicTypeBuilder::set_name(const std::string& name)
{
    if (descriptor_ != nullptr)
    {
        descriptor_->set_name(name);
    }
    name_ = name;
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicTypeBuilder::_apply_annotation_to_member(
        MemberId id,
        AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        auto it = member_by_id_.find(id);
        if (it != member_by_id_.end())
        {
            it->second->apply_annotation(descriptor);
            return ResponseCode::RETCODE_OK;
        }
        else
        {
            logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ResponseCode::RETCODE_BAD_PARAMETER;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation to member. The input descriptor isn't consistent.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicTypeBuilder::_apply_annotation_to_member(
        MemberId id,
        const std::string& key,
        const std::string& value)
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        it->second->apply_annotation(key, value);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
