// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/v1_3/DynamicDataFactory.hpp>
#include <fastrtps/types/v1_3/MemberDescriptor.hpp>
#include <fastrtps/types/v1_3/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps::types::v1_3;

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

ReturnCode_t DynamicDataFactory::delete_instance()
{
    if (s_instance != nullptr)
    {
        delete s_instance;
        s_instance = nullptr;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
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
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
}

DynamicData* DynamicDataFactory::create_copy(
        const DynamicData* pData)
{
    DynamicData* newData = new DynamicData(pData);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    {
        std::unique_lock<std::recursive_mutex> scoped(mutex_);
        dynamic_datas_.push_back(newData);
    }
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK

    return newData;
}

DynamicData* DynamicDataFactory::create_data(
        const DynamicTypeBuilder& builder)
{
    if (builder.is_consistent())
    {
        DynamicType_ptr type{builder.build()};
        return create_data(*type);
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating DynamicData. Invalid dynamic type builder");
        return nullptr;
    }
}

DynamicData* DynamicDataFactory::create_data(
        const DynamicType& type)
{
    if (type.is_consistent())
    {
        try
        {
            DynamicData* newData = nullptr;
            // ALIAS types create a DynamicData based on the base type and renames it with the name of the ALIAS.
            DynamicType_ptr base_type{type.get_base_type()};
            if (base_type)
            {
                if (type.get_kind() == TypeKind::TK_ALIAS)
                {
                    newData = create_data(*base_type);
                    // newData->set_type_name(type.get_name());
                }
                else if (type.get_kind() == TypeKind::TK_STRUCTURE || type.get_kind() == TypeKind::TK_BITSET)
                {
                    newData = new DynamicData(type);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mutex_);
                        dynamic_datas_.push_back(newData);
                    }
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                }
            }
            else
            {
                newData = new DynamicData(type);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                {
                    std::unique_lock<std::recursive_mutex> scoped(mutex_);
                    dynamic_datas_.push_back(newData);
                }
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK

                // Enums must have a default value
                if (type.get_kind() == TypeKind::TK_ENUM)
                {
                    MemberId id = type.get_member_id_at_index(0);
                    // enums cannot be instantiated without members
                    assert(MEMBER_ID_INVALID != id);
                    // initialize the enum
                    newData->set_uint32_value(*id);
                }

                // Arrays must have created every members for serialization.
                if (type.get_kind() == TypeKind::TK_ARRAY)
                {
                    DynamicType_ptr element_type{type.get_element_type()};
                    DynamicData* defaultArrayData = new DynamicData(*element_type);
                    assert(nullptr != defaultArrayData);
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    {
                        std::unique_lock<std::recursive_mutex> scoped(mutex_);
                        dynamic_datas_.push_back(defaultArrayData);
                    }
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
                    newData->default_array_value_ = defaultArrayData;
                }
            }
            return newData;
        }
        catch (std::exception& e)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Exception creating DynamicData: " << e.what());
            return nullptr;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error creating DynamicData. Invalid dynamic type");
        return nullptr;
    }
}

ReturnCode_t DynamicDataFactory::create_members(
        DynamicData* pData,
        DynamicType_ptr pType)
{
    if (pType && pData != nullptr)
    {
        pData->create_members(*pType);
        DynamicType_ptr base_type {pType->get_base_type()};
        if ((pType->get_kind() == TypeKind::TK_STRUCTURE || pType->get_kind() == TypeKind::TK_BITSET) && base_type)
        {
            create_members(pData, base_type);
        }
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_BAD_PARAMETER;
}

ReturnCode_t DynamicDataFactory::delete_data(
        DynamicData* pData)
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
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error deleting DynamicData. It isn't registered in the factory");
            return ReturnCode_t::RETCODE_ALREADY_DELETED;
        }
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
        delete pData;
    }
    return ReturnCode_t::RETCODE_OK;
}

bool DynamicDataFactory::is_empty() const
{
#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::unique_lock<std::recursive_mutex> scoped(mutex_);
    return dynamic_datas_.empty();
#else
    return true;
#endif // ifndef DISABLE_DYNAMIC_MEMORY_CHECK
}
