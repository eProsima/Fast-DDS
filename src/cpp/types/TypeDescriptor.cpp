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

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {

TypeDescriptor::TypeDescriptor()
    : mKind(0)
    , mName("")
    , mBaseType(nullptr)
    , mDiscriminatorType(nullptr)
    , mElementType(nullptr)
    , mKeyElementType(nullptr)
{
}

TypeDescriptor::TypeDescriptor(const std::string& name, TypeKind kind)
    : mKind(kind)
    , mName(name)
    , mBaseType(nullptr)
    , mDiscriminatorType(nullptr)
    , mElementType(nullptr)
    , mKeyElementType(nullptr)
{
}

TypeDescriptor::TypeDescriptor(const TypeDescriptor* other)
    : mKind(0)
    , mName("")
    , mBaseType(nullptr)
    , mDiscriminatorType(nullptr)
    , mElementType(nullptr)
    , mKeyElementType(nullptr)
{
    CopyFrom(other);
}

TypeDescriptor::~TypeDescriptor()
{
    Clean();
}

void TypeDescriptor::Clean()
{
    mBaseType = nullptr;
    mDiscriminatorType = nullptr;
    mElementType = nullptr;
    mKeyElementType = nullptr;
}

ResponseCode TypeDescriptor::CopyFrom(const TypeDescriptor* descriptor)
{
    if (descriptor != nullptr)
    {
        try
        {
            Clean();

            mKind = descriptor->mKind;
            mName = descriptor->mName;
            mBaseType = descriptor->mBaseType;
            mDiscriminatorType = descriptor->mDiscriminatorType;
            mBound = descriptor->mBound;
            mElementType = descriptor->mElementType;
            mKeyElementType = descriptor->mKeyElementType;
            return ResponseCode::RETCODE_OK;
        }
        catch (std::exception& /*e*/)
        {
            return ResponseCode::RETCODE_ERROR;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error copying TypeDescriptor, invalid input descriptor");
        return ResponseCode::RETCODE_BAD_PARAMETER;
    }
}

bool TypeDescriptor::Equals(const TypeDescriptor* descriptor) const
{
    return descriptor != nullptr && mName == descriptor->mName && mKind == descriptor->mKind &&
        mBaseType == descriptor->mBaseType && mDiscriminatorType == descriptor->mDiscriminatorType &&
        mBound == descriptor->mBound && mElementType == descriptor->mElementType &&
        mKeyElementType == descriptor->mKeyElementType;
}

DynamicType_ptr TypeDescriptor::GetBaseType() const
{
    return mBaseType;
}

uint32_t TypeDescriptor::GetBounds(uint32_t index /*=0*/) const
{
    if (index < mBound.size())
    {
        return mBound[index];
    }
    else
    {
        logError(DYN_TYPES, "Error getting bounds value. Index out of range.");
        return LENGTH_UNLIMITED;
    }
}

uint32_t TypeDescriptor::GetBoundsSize() const
{
    return static_cast<uint32_t>(mBound.size());
}

DynamicType_ptr TypeDescriptor::GetDiscriminatorType() const
{
    return mDiscriminatorType;
}

DynamicType_ptr TypeDescriptor::GetElementType() const
{
    return mElementType;
}

DynamicType_ptr TypeDescriptor::GetKeyElementType() const
{
    return mKeyElementType;
}

TypeKind TypeDescriptor::GetKind() const
{
    return mKind;
}

std::string TypeDescriptor::GetName() const
{
    return mName;
}

uint32_t TypeDescriptor::GetTotalBounds() const
{
    if (mBound.size() >= 1)
    {
        uint32_t bounds = 1;
        for (uint32_t i = 0; i < mBound.size(); ++i)
        {
            bounds *= mBound[i];
        }
        return bounds;
    }
    return LENGTH_UNLIMITED;
}

bool TypeDescriptor::IsConsistent() const
{
    // Alias Types need the base type to indicate what type has been aliased.
    if (mKind == TK_ALIAS && mBaseType == nullptr)
    {
        return false;
    }

    // Alias must have base type and structures optionally can have it.
    if (mBaseType != nullptr && mKind != TK_ALIAS && mKind != TK_STRUCTURE)
    {
        return false;
    }

    // Arrays need one or more bound fields with the lenghts of each dimension.
    if (mKind == TK_ARRAY && mBound.size() == 0)
    {
        return false;
    }

    // These types need one bound with the length of the field.
    if (mBound.size() != 1 && (mKind == TK_SEQUENCE || mKind == TK_MAP || mKind == TK_BITMASK ||
        mKind == TK_STRING8 || mKind == TK_STRING16))
    {
        return false;
    }

    // Only union types need the discriminator of the union
    if ((mDiscriminatorType == nullptr) == (mKind == TK_UNION))
    {
        return false;
    }

    // ElementType is used by these types to set the "value" type of the element, otherwise it should be null.
    if ((mElementType == nullptr) == (mKind == TK_ARRAY || mKind == TK_SEQUENCE || mKind == TK_STRING8 ||
        mKind == TK_STRING16 || mKind == TK_MAP || mKind == TK_BITMASK))
    {
        return false;
    }

    // For Bitmask types is mandatory that this element is boolean.
    if (mKind == TK_BITMASK && (mElementType->GetKind() != TK_BOOLEAN))
    {
        return false;
    }

    // Only map types need the keyElementType to store the "Key" type of the pair.
    if ((mKeyElementType == nullptr) == (mKind == TK_MAP))
    {
        return false;
    }

    if (!IsTypeNameConsistent(mName))
    {
        return false;
    }

    return true;
}

bool TypeDescriptor::IsTypeNameConsistent(const std::string& sName) const
{
    // The first letter must start with a letter ( uppercase or lowercase )
    if (sName.length() > 0 && std::isalpha(sName[0]))
    {
        // All characters must be letters, numbers or underscore.
        for (uint32_t i = 1; i < sName.length(); ++i)
        {
            if (!std::isalnum(sName[i]) && sName[i] != 95)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}


void TypeDescriptor::SetKind(TypeKind kind)
{
    mKind = kind;
}

void TypeDescriptor::SetName(std::string name)
{
    mName = name;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
