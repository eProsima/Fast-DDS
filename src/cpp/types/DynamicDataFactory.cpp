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
        DynamicDataFactory::delete_instance();
    }
};

static DynamicDataFactoryReleaser s_releaser;
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
    std::unique_lock<std::recursive_mutex> scoped(mutex_);
    while (dynamic_datas_.size() > 0)
    {
        delete_data(dynamic_datas_[dynamic_datas_.size() - 1]);
    }
    dynamic_datas_.clear();
#endif
}

DynamicData* DynamicDataFactory::create_copy(const DynamicData* pData)
{
    DynamicData* newData = new DynamicData(pData);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    {
        std::unique_lock<std::recursive_mutex> scoped(mutex_);
        dynamic_datas_.push_back(newData);
    }
#endif

    return newData;
}

DynamicData* DynamicDataFactory::create_data(DynamicTypeBuilder* pBuilder)
{
    if (pBuilder != nullptr && pBuilder->is_consistent())
    {
        DynamicType_ptr pType = DynamicTypeBuilderFactory::get_instance()->create_type(pBuilder);
        return create_data(pType);
    }
    else
    {
        logError(DYN_TYPES, "Error creating DynamicData. Invalid dynamic type builder");
        return nullptr;
    }
}

DynamicData* DynamicDataFactory::create_data(DynamicType_ptr pType)
{
    if (pType != nullptr && pType->is_consistent())
    {
        try
        {
            DynamicData* newData = nullptr;
            // ALIAS types create a DynamicData based on the base type and renames it with the name of the ALIAS.
            if (pType->get_base_type() != nullptr)
            {
                if (pType->get_kind() == TK_ALIAS)
                {
                    newData = create_data(pType->get_base_type());
                    newData->set_type_name(pType->get_name());
                }
                else if (pType->get_kind() == TK_STRUCTURE)
                {
                    newData = new DynamicData(pType);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mutex_);
                        dynamic_datas_.push_back(newData);
                    }
#endif
                    create_members(newData, pType->get_base_type());
                }
            }
            else
            {
                newData = new DynamicData(pType);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                {
                    std::unique_lock<std::recursive_mutex> scoped(mutex_);
                    dynamic_datas_.push_back(newData);
                }
#endif

                // Arrays must have created every members for serialization.
                if (pType->get_kind() == TK_ARRAY)
                {
                    DynamicData* defaultArrayData = new DynamicData(pType->get_element_type());
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mutex_);
                        dynamic_datas_.push_back(defaultArrayData);
                    }
#endif
                    newData->default_array_value_ = defaultArrayData;
                }
                // Unions need a discriminator data
                else if (pType->get_kind() == TK_UNION)
                {
                    DynamicData* discriminatorData = new DynamicData(pType->get_discriminator_type());
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mutex_);
                        dynamic_datas_.push_back(discriminatorData);
                    }
#endif
                    newData->set_union_discriminator(discriminatorData);
                }
            }
            return newData;
        }
        catch (std::exception& e)
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

ResponseCode DynamicDataFactory::create_members(
        DynamicData* pData,
        DynamicType_ptr pType)
{
    if (pType != nullptr && pData != nullptr)
    {
        pData->create_members(pType);
        if (pType->get_kind() == TK_STRUCTURE && pType->get_base_type() != nullptr)
        {
            create_members(pData, pType->get_base_type());
        }
        return ResponseCode::RETCODE_OK;
    }
    return ResponseCode::RETCODE_BAD_PARAMETER;
}

ResponseCode DynamicDataFactory::delete_data(DynamicData* pData)
{
    if (pData != nullptr)
    {
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        std::unique_lock<std::recursive_mutex> scoped(mutex_);
        auto it = std::find(dynamic_datas_.begin(), dynamic_datas_.end(), pData);
        if (it != dynamic_datas_.end())
        {
            dynamic_datas_.erase(it);
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

bool DynamicDataFactory::is_empty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mutex_);
    return dynamic_datas_.empty();
#else
    return true;
#endif
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
