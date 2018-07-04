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
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicType::DynamicType()
    : mDescriptor(nullptr)
    , mName("")
    , mKind(TK_NONE)
{
}

DynamicType::DynamicType(const TypeDescriptor* descriptor)
{
    mDescriptor = new TypeDescriptor(descriptor);
    try
    {
        mName = descriptor->getName();
        mKind = descriptor->getKind();
    }
    catch (...)
    {
        mName = "";
        mKind = TK_NONE;
    }

    // Alias types use the same members than it's base class.
    if (mKind == TK_ALIAS)
    {
        for (auto it = mDescriptor->getBaseType()->mMemberById.begin();
            it != mDescriptor->getBaseType()->mMemberById.end(); ++it)
        {
            mMemberByName.insert(std::make_pair(it->second->get_name(), it->second));
        }
    }

    if (is_complex_kind())
    {
        //TODO: //ARCE: FILL MEMBERS
    }
}

DynamicType::DynamicType(const DynamicType* other)
    : mDescriptor(nullptr)
    , mName("")
    , mKind(TK_NONE)
{
    copy_from_type(other);
}

DynamicType::~DynamicType()
{
    Clear();
}

void DynamicType::Clear()
{
    mName = "";
    mKind = 0;
    if (mDescriptor != nullptr)
    {
        delete mDescriptor;
        mDescriptor = nullptr;
    }

    for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
    {
        delete *it;
    }
    mAnnotation.clear();

    for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
    {
        delete it->second;
    }
    mMemberById.clear();
    mMemberByName.clear();
}

ResponseCode DynamicType::copy_from_type(const DynamicType* other)
{
    if (other != nullptr)
    {
        Clear();

        mName = other->mName;
        mKind = other->mKind;
        mDescriptor = new TypeDescriptor(other->mDescriptor);

        for (auto it = other->mAnnotation.begin(); it != other->mAnnotation.end(); ++it)
        {
            AnnotationDescriptor* newDescriptor = new AnnotationDescriptor(*it);
            mAnnotation.push_back(newDescriptor);
        }

        for (auto it = other->mMemberById.begin(); it != other->mMemberById.end(); ++it)
        {
            DynamicTypeMember* newMember = new DynamicTypeMember(it->second);
            mMemberById.insert(std::make_pair(newMember->get_id(), newMember));
            mMemberByName.insert(std::make_pair(newMember->get_name(), newMember));
        }
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error copying DynamicType, invalid input type");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

ResponseCode DynamicType::get_descriptor(TypeDescriptor* descriptor) const
{
    if (descriptor != nullptr)
    {
        descriptor->copy_from(mDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logError(DYN_TYPES, "Error getting TypeDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool DynamicType::equals(const DynamicType* other) const
{
    if (other != nullptr && mAnnotation.size() == other->mAnnotation.size() &&
        mMemberById.size() == other->mMemberById.size() && mMemberByName.size() == other->mMemberByName.size())
    {
        // Check the annotation list
        for (auto it = mAnnotation.begin(), it2 = other->mAnnotation.begin(); it != mAnnotation.end(); ++it, ++it2)
        {
            if (!(*it)->equals(*it))
            {
                return false;
            }
        }

        // Check the members by Id
        for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
        {
            auto it2 = other->mMemberById.find(it->first);
            if (it2 == other->mMemberById.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        for (auto it = other->mMemberById.begin(); it != other->mMemberById.end(); ++it)
        {
            auto it2 = mMemberById.find(it->first);
            if (it2 == mMemberById.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        // Check the members by Name
        for (auto it = mMemberByName.begin(); it != mMemberByName.end(); ++it)
        {
            auto it2 = other->mMemberByName.find(it->first);
            if (it2 == other->mMemberByName.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        for (auto it = other->mMemberByName.begin(); it != other->mMemberByName.end(); ++it)
        {
            auto it2 = mMemberByName.find(it->first);
            if (it2 == mMemberByName.end() || !it2->second->equals(it->second))
            {
                return false;
            }
        }

        return true;
    }
    return false;
}

std::string DynamicType::get_name() const
{
    return mName;
}

TypeKind DynamicType::get_kind() const
{
    return mKind;
}

ResponseCode DynamicType::get_member_by_name(DynamicTypeMember& member, const std::string name)
{
    auto it = mMemberByName.find(name);
    if (it != mMemberByName.end())
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
    members = mMemberByName;
    return ResponseCode::RETCODE_OK;
}

ResponseCode DynamicType::get_member(DynamicTypeMember& member, MemberId id)
{
    auto it = mMemberById.find(id);
    if (it != mMemberById.end())
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
    members = mMemberById;
    return ResponseCode::RETCODE_OK;
}

uint32_t DynamicType::get_annotation_count()
{
    return static_cast<uint32_t>(mAnnotation.size());
}

ResponseCode DynamicType::get_annotation(AnnotationDescriptor& descriptor, uint32_t idx)
{
    if (idx < mAnnotation.size())
    {
        descriptor = *mAnnotation[idx];
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        logWarning(DYN_TYPES, "Error getting annotation, annotation not found.");
        return ResponseCode::RETCODE_ERROR;
    }
}

DynamicType* DynamicType::getBaseType() const
{
    if (mDescriptor != nullptr)
    {
        return mDescriptor->getBaseType();
    }
    return nullptr;
}

uint32_t DynamicType::get_bounds(uint32_t index /*= 0*/) const
{
    if (mDescriptor != nullptr)
    {
        return mDescriptor->getBounds(index);
    }
    return LENGTH_UNLIMITED;
}

DynamicType* DynamicType::getElementType() const
{
    if (mDescriptor != nullptr)
    {
        return mDescriptor->getElementType();
    }
    return nullptr;
}

DynamicType* DynamicType::getKeyElementType() const
{
    if (mDescriptor != nullptr)
    {
        return mDescriptor->getKeyElementType();
    }
    return nullptr;
}

bool DynamicType::is_complex_kind() const
{
    return mKind == TK_ANNOTATION || mKind == TK_ARRAY || mKind == TK_BITMASK || mKind == TK_BITSET
        || mKind == TK_MAP || mKind == TK_SEQUENCE || mKind == TK_STRUCTURE;
}

void DynamicType::SetName(const std::string& name)
{
    if (mDescriptor != nullptr)
    {
        mDescriptor->setName(name);
    }
    mName = name;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
