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
DynamicDataFactory* DynamicDataFactory::get_instance()
{
    if (s_instance == nullptr)
    {
        s_instance = new DynamicDataFactory();
    }
    return s_instance;
}

ResponseCode DynamicDataFactory::delete_instance()
{
    if (s_instance != nullptr)
    {
        delete s_instance;
        s_instance = nullptr;
    }
    return ResponseCode::RETCODE_OK;
}

DynamicDataFactory::DynamicDataFactory()
{
}

DynamicDataFactory::~DynamicDataFactory()
{
    for (auto it = mDynamicDatas.begin(); it != mDynamicDatas.end(); ++it)
        delete *it;
    mDynamicDatas.clear();
}

DynamicData* DynamicDataFactory::create_data(DynamicType* pType)
{
    if (pType != nullptr)
    {
        try
        {
            DynamicData* newData = nullptr;
            // ALIAS types create a DynamicData based on the base type and renames it with the name of the ALIAS.
            if (pType->get_kind() == TK_ALIAS)
            {
                newData = new DynamicData(pType->getBaseType());
                newData->SetTypeName(pType->get_name());
            }
            else
            {
                newData = new DynamicData(pType);
            }

            mDynamicDatas.push_back(newData);
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

ResponseCode DynamicDataFactory::delete_data(DynamicData* data)
{
    if (data != nullptr)
    {
        auto it = std::find(mDynamicDatas.begin(), mDynamicDatas.end(), data);
        if (it != mDynamicDatas.end())
        {
            mDynamicDatas.erase(it);
            delete data;
        }
        else
        {
            logError(DYN_TYPES, "Error deleting DynamicData. It isn't registered in the factory");
            return ResponseCode::RETCODE_ALREADY_DELETED;
        }
    }
    return ResponseCode::RETCODE_OK;
}

bool DynamicDataFactory::IsEmpty() const
{
    return mDynamicDatas.empty();
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
