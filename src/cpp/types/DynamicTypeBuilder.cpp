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
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/AnnotationDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilder::DynamicTypeBuilder()
    : mCurrentMemberId(0)
{
}

DynamicTypeBuilder::DynamicTypeBuilder(const TypeDescriptor* descriptor)
    : DynamicType(descriptor)
    , mCurrentMemberId(0)
{
}

DynamicTypeBuilder::DynamicTypeBuilder(const DynamicType* pType)
    : DynamicType()
    , mCurrentMemberId(0)
{
    if (pType != nullptr)
    {
        copy_from_type(pType);
    }
}

ResponseCode DynamicTypeBuilder::add_member(const MemberDescriptor* descriptor)
{
    if (mDescriptor->getKind() == TK_ANNOTATION || mDescriptor->getKind() == TK_ALIAS
        || mDescriptor->getKind() == TK_BITMASK || mDescriptor->getKind() == TK_ENUM
        || mDescriptor->getKind() == TK_STRUCTURE || mDescriptor->getKind() == TK_UNION)
    {
        DynamicTypeMember* newMember = new DynamicTypeMember(descriptor);
        mMemberById.insert(std::make_pair(mCurrentMemberId, newMember));
        mMemberByName.insert(std::make_pair(newMember->get_name(), newMember));
        ++mCurrentMemberId;
        return ResponseCode::RETCODE_OK;
    }
    else
    {
        return ResponseCode::RETCODE_PRECONDITION_NOT_MET;
    }
}

ResponseCode DynamicTypeBuilder::apply_annotation(AnnotationDescriptor& descriptor)
{
    if (descriptor.isConsistent())
    {
        AnnotationDescriptor* pNewDescriptor = new AnnotationDescriptor();
        pNewDescriptor->copy_from(&descriptor);
        mAnnotation.push_back(pNewDescriptor);
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicTypeBuilder::apply_annotation_to_member(MemberId id, AnnotationDescriptor& descriptor)
{
    if (descriptor.isConsistent())
    {
        auto it = mMemberById.find(id);
        if (it != mMemberById.end())
        {
            it->second->apply_annotation(descriptor);
            return ResponseCode::RETCODE_OK;
        }
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

DynamicType* DynamicTypeBuilder::build()
{
    DynamicType* newType = new DynamicType(mDescriptor);
    newType->mName = mDescriptor->getName();
    newType->mKind = mDescriptor->getKind();
    for (auto it = mAnnotation.begin(); it != mAnnotation.end(); ++it)
    {
        AnnotationDescriptor* newAnnotation = new AnnotationDescriptor();
        newAnnotation->copy_from(*it);
        newType->mAnnotation.push_back(newAnnotation);
    }

    for (auto it = mMemberById.begin(); it != mMemberById.end(); ++it)
    {
        newType->mMemberById.insert(std::make_pair(it->first, it->second));
    }

    for (auto it = mMemberByName.begin(); it != mMemberByName.end(); ++it)
    {
        newType->mMemberByName.insert(std::make_pair(it->first, it->second));
    }
    return newType;
}

ResponseCode DynamicTypeBuilder::copy_from(const DynamicTypeBuilder* other)
{
    if (other != nullptr)
    {
        Clear();

        ResponseCode res = copy_from_type(other);
        if (res == ResponseCode::RETCODE_OK)
        {
            mCurrentMemberId = other->mCurrentMemberId;
        }
        return res;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

void DynamicTypeBuilder::Clear()
{
    DynamicType::Clear();

    mCurrentMemberId = 0;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
