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
#include <dynamic-types/v1_3/DynamicTypeImpl.hpp>
#include <dynamic-types/v1_3/DynamicDataImpl.hpp>
#include <dynamic-types/v1_3/DynamicDataFactoryImpl.hpp>

using namespace eprosima::fastrtps::types::v1_3;

DynamicDataFactory& DynamicDataFactory::get_instance()
{
    // C++11 guarantees the construction to be atomic
    static DynamicDataFactory instance;
    return instance;
}

ReturnCode_t DynamicDataFactory::delete_instance()
{
    // Delegate into the implementation class
    return DynamicDataFactoryImpl::delete_instance();
}

DynamicData* DynamicDataFactory::create_data(
        const DynamicType& type)
{
    // Delegate into the implementation class
    auto data = DynamicDataFactoryImpl::get_instance().create_data(DynamicTypeImpl::get_implementation(type));
    return &data->get_interface();

}

DynamicData* DynamicDataFactory::create_copy(
        const DynamicData& data)
{
    return &DynamicDataFactoryImpl::get_instance()
            .create_copy(DynamicDataImpl::get_implementation(data))->get_interface();
}

ReturnCode_t DynamicDataFactory::delete_data(
        const DynamicData* pData)
{
    if (nullptr == pData)
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }

    const auto& ti = DynamicDataImpl::get_implementation(*pData);
    return DynamicDataFactoryImpl::get_instance().delete_data(ti);
}
