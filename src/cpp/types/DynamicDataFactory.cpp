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

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicDataFactoryReleaser
{
public:
    ~DynamicDataFactoryReleaser()
    {
        DynamicDataFactory::DeleteInstance();
    }
};

static DynamicDataFactoryReleaser s_releaser;
static DynamicDataFactory* s_instance = nullptr;

DynamicDataFactory* DynamicDataFactory::GetInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new DynamicDataFactory();
    }
    return s_instance;
}

ResponseCode DynamicDataFactory::DeleteInstance()
{
    if (s_instance != nullptr)
    {
        delete s_instance;
        s_instance = nullptr;
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_ERROR;
}

DynamicDataFactory::DynamicDataFactory()
{
}

DynamicDataFactory::~DynamicDataFactory()
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mMutex);
    while (mDynamicDatas.size() > 0)
    {
        DeleteData(mDynamicDatas[mDynamicDatas.size() - 1]);
    }
    mDynamicDatas.clear();
#endif
}

DynamicData* DynamicDataFactory::CreateCopy(const DynamicData* pData)
{
    DynamicData* newData = new DynamicData(pData);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    {
        std::unique_lock<std::recursive_mutex> scoped(mMutex);
        mDynamicDatas.push_back(newData);
    }
#endif

    return newData;
}

DynamicData* DynamicDataFactory::CreateData(DynamicTypeBuilder* pBuilder)
{
    if (pBuilder != nullptr && pBuilder->IsConsistent())
    {
        DynamicType_ptr pType = DynamicTypeBuilderFactory::GetInstance()->CreateType(pBuilder);
        return CreateData(pType);
    }
    else
    {
        logError(DYN_TYPES, "Error creating DynamicData. Invalid dynamic type builder");
        return nullptr;
    }
}

DynamicData* DynamicDataFactory::CreateData(DynamicType_ptr pType)
{
    if (pType != nullptr && pType->IsConsistent())
    {
        try
        {
            DynamicData* newData = nullptr;
            // ALIAS types create a DynamicData based on the base type and renames it with the name of the ALIAS.
            if (pType->GetBaseType() != nullptr)
            {
                if (pType->GetKind() == TK_ALIAS)
                {
                    newData = CreateData(pType->GetBaseType());
                    newData->SetTypeName(pType->GetName());
                }
                else if (pType->GetKind() == TK_STRUCTURE)
                {
                    newData = new DynamicData(pType);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mMutex);
                        mDynamicDatas.push_back(newData);
                    }
#endif
                    CreateMembers(newData, pType->GetBaseType());
                }
            }
            else
            {
                newData = new DynamicData(pType);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                {
                    std::unique_lock<std::recursive_mutex> scoped(mMutex);
                    mDynamicDatas.push_back(newData);
                }
#endif

                // Arrays must have created every members for serialization.
                if (pType->GetKind() == TK_ARRAY)
                {
                    DynamicData* defaultArrayData = new DynamicData(pType->GetElementType());
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mMutex);
                        mDynamicDatas.push_back(defaultArrayData);
                    }
#endif
                    newData->mDefaultArrayValue = defaultArrayData;
                }
                // Unions need a discriminator data
                else if (pType->GetKind() == TK_UNION)
                {
                    DynamicData* discriminatorData = new DynamicData(pType->GetDiscriminatorType());
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mMutex);
                        mDynamicDatas.push_back(discriminatorData);
                    }
#endif
                    newData->SetUnionDiscriminator(discriminatorData);
                }
            }
            return newData;
        }
        catch (std::exception &e)
        {
            logError(DYN_TYPES, "Exception creating DynamicData: " << e.what());
            return nullptr;
        }
    }
    else
    {
        logError(DYN_TYPES, "Error creating DynamicData. Invalid dynamic type");
        return nullptr;
    }
}

ResponseCode DynamicDataFactory::CreateMembers(DynamicData* pData, DynamicType_ptr pType)
{
    if (pType != nullptr && pData != nullptr)
    {
        pData->CreateMembers(pType);
        if (pType->GetKind() == TK_STRUCTURE && pType->GetBaseType() != nullptr)
        {
            CreateMembers(pData, pType->GetBaseType());
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicDataFactory::DeleteData(DynamicData* pData)
{
    if (pData != nullptr)
    {
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        std::unique_lock<std::recursive_mutex> scoped(mMutex);
        auto it = std::find(mDynamicDatas.begin(), mDynamicDatas.end(), pData);
        if (it != mDynamicDatas.end())
        {
            mDynamicDatas.erase(it);
        }
        else
        {
            logError(DYN_TYPES, "Error deleting DynamicData. It isn't registered in the factory");
            return ResponseCode::RETCODE_ALREADY_DELETED;
        }
#endif
        delete pData;
    }
    return ResponseCode::RETCODE_OK;
}

bool DynamicDataFactory::IsEmpty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mMutex);
    return mDynamicDatas.empty();
#else
    return true;
#endif
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
