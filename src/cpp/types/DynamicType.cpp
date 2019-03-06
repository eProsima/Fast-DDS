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
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicData.h>

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

DynamicType::DynamicType(const TypeDescriptor* descriptor)
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

DynamicType::DynamicType(const DynamicTypeBuilder* other)
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

ResponseCode DynamicType::apply_annotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.is_consistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        annotation_.push_back(pNewDescriptor);
        is_key_defined_ = key_annotation();
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error applying annotation. The input descriptor isn't consistent.");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicType::apply_annotation(const std::string& key, const std::string& value)
{
    auto it = annotation_.begin();
    if (it != annotation_.end())
    {
        (*it)->set_value(key, value);
    }
    else
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->set_type(DynamicTypeBuilderFactory::get_instance()->create_annotation_primitive());
        pNewDescriptor->set_value(key, value);
        annotation_.push_back(pNewDescriptor);
        is_key_defined_ = key_annotation();
    }

    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::apply_annotation_to_member(
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

ResponseCode DynamicType::apply_annotation_to_member(
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

void DynamicType::clear()
{
    name_ = "";
    kind_ = 0;
    if (descriptor_ != nullptr)
    {
        delete descriptor_;
        descriptor_ = nullptr;
    }

    for (auto it = annotation_.begin(); it != annotation_.end(); ++it)
    {
        delete *it;
    }
    annotation_.clear();

    for (auto it = member_by_id_.begin(); it != member_by_id_.end(); ++it)
    {
        delete it->second;
    }
    member_by_id_.clear();
    member_by_name_.clear();
}

ResponseCode DynamicType::copy_from_builder(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        clear();

        name_ = other->name_;
        kind_ = other->kind_;
        descriptor_ = new TypeDescriptor(other->descriptor_);

        for (auto it = other->annotation_.begin(); it != other->annotation_.end(); ++it)
        {
            AnnotationDescriptor* newDescriptor = new AnnotationDescriptor(*it);
            annotation_.push_back(newDescriptor);
        }

        for (auto it = other->member_by_id_.begin(); it != other->member_by_id_.end(); ++it)
        {
            DynamicTypeMember* newMember = new DynamicTypeMember(it->second);
            newMember->set_parent(this);
            is_key_defined_ = newMember->key_annotation();
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

bool DynamicType::exists_member_by_name(const std::string& name) const
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

ResponseCode DynamicType::get_descriptor(TypeDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(descriptor_);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting TypeDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicType::key_annotation() const
{
    for (auto anIt = annotation_.begin(); anIt != annotation_.end(); ++anIt)
    {
        if ((*anIt)->key_annotation())
        {
            return true;
        }
    }
    return false;
}

bool DynamicType::equals(const DynamicType* other) const
{
    if (other != nullptr && annotation_.size() == other->annotation_.size() &&
        member_by_id_.size() == other->member_by_id_.size() && member_by_name_.size() == other->member_by_name_.size())
    {
        // Check the annotation list
        for (auto it = annotation_.begin(), it2 = other->annotation_.begin(); it != annotation_.end(); ++it, ++it2)
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

ResponseCode DynamicType::get_member_by_name(
        DynamicTypeMember& member,
        const std::string& name)
{
    auto it = member_by_name_.find(name);
    if (it != member_by_name_.end())
    {
        member = it->second;
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting member by name, member not found.");
        return ResponseCode::RETCODE_ERROR;
    }
}

ResponseCode DynamicType::get_all_members_by_name(std::map<std::string, DynamicTypeMember*>& members)
{
    members = member_by_name_;
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::get_member(
        DynamicTypeMember& member,
        MemberId id)
{
    auto it = member_by_id_.find(id);
    if (it != member_by_id_.end())
    {
        member = it->second;
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting member, member not found.");
        return ResponseCode::RETCODE_ERROR;
    }
}

ResponseCode DynamicType::get_all_members(std::map<MemberId, DynamicTypeMember*>& members)
{
    members = member_by_id_;
    return ResponseCode::RETCODE_OK;
}

uint32_t DynamicType::get_annotation_count()
{
    return static_cast<uint32_t>(annotation_.size());
}

ResponseCode DynamicType::get_annotation(
        AnnotationDescriptor& descriptor,
        uint32_t idx)
{
    if (idx < annotation_.size())
    {
        descriptor = *annotation_[idx];
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting annotation, annotation not found.");
        return ResponseCode::RETCODE_ERROR;
    }
}

DynamicType_ptr DynamicType::get_base_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_base_type();
    }
    return nullptr;
}

uint32_t DynamicType::get_bounds(uint32_t index /*= 0*/) const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_bounds(index);
    }
    return LENGTH_UNLIMITED;
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
    return nullptr;
}

DynamicType_ptr DynamicType::get_element_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_element_type();
    }
    return nullptr;
}

DynamicType_ptr DynamicType::get_key_element_type() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_key_element_type();
    }
    return nullptr;
}

uint32_t DynamicType::get_total_bounds() const
{
    if (descriptor_ != nullptr)
    {
        return descriptor_->get_total_bounds();
    }
    return LENGTH_UNLIMITED;
}

bool DynamicType::has_children() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_MAP || kind_ == TK_SEQUENCE
        || kind_ == TK_STRUCTURE || kind_ == TK_UNION;
}

bool DynamicType::is_complex_kind() const
{
    return kind_ == TK_ANNOTATION || kind_ == TK_ARRAY || kind_ == TK_BITMASK || kind_ == TK_ENUM
        || kind_ == TK_MAP || kind_ == TK_SEQUENCE || kind_ == TK_STRUCTURE || kind_ == TK_UNION;
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

void DynamicType::set_name(const std::string& name)
{
    if (descriptor_ != nullptr)
    {
        descriptor_->set_name(name);
    }
    name_ = name;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
