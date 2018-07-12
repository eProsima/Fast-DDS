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
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace types {

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
    for (auto it = mDynamicDatas.begin(); it != mDynamicDatas.end(); ++it)
        delete *it;
    mDynamicDatas.clear();
#endif
}

DynamicData* DynamicDataFactory::CreateData(DynamicType* pType)
{
    if (pType != nullptr && pType->IsTypeObject())
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
                    mDynamicDatas.push_back(newData);

                    CreateMembers(newData, pType->GetBaseType());
                }
            }
            else
            {
                newData = new DynamicData(pType);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                mDynamicDatas.push_back(newData);
#endif

                // Arrays must have created every members for serialization.
                if (pType->GetKind() == TK_ARRAY)
                {
                    DynamicData* defaultArrayData = new DynamicData(pType->GetElementType());
                    mDynamicDatas.push_back(defaultArrayData);
                    newData->mDefaultArrayValue = defaultArrayData;
                }
            }
            return newData;
        }
        catch (std::exception e)
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

ResponseCode DynamicDataFactory::CreateMembers(DynamicData* pData, DynamicType* pType)
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
        auto it = std::find(mDynamicDatas.begin(), mDynamicDatas.end(), pData);
        if (it != mDynamicDatas.end())
        {
            mDynamicDatas.erase(it);
            delete pData;
        }
        else
        {
            logError(DYN_TYPES, "Error deleting DynamicData. It isn't registered in the factory");
            return ResponseCode::RETCODE_ALREADY_DELETED;
        }
#else
        delete data;
#endif
    }
    return ResponseCode::RETCODE_OK;
}

bool DynamicDataFactory::IsEmpty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    return mDynamicDatas.empty();
#else
    return true;
#endif
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
