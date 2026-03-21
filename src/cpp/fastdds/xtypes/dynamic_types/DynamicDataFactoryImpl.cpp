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

#include "DynamicDataFactoryImpl.hpp"

#include "DynamicDataImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

traits<DynamicDataFactoryImpl>::ref_type DynamicDataFactoryImpl::instance_;

traits<DynamicDataFactory>::ref_type DynamicDataFactoryImpl::get_instance() noexcept
{
    if (!instance_)
    {
        instance_ = std::make_shared<DynamicDataFactoryImpl>();
    }

    return instance_;
}

ReturnCode_t DynamicDataFactoryImpl::delete_instance() noexcept
{
    if (!instance_)
    {
        return RETCODE_BAD_PARAMETER;
    }
    instance_.reset();
    return RETCODE_OK;
}

traits<DynamicData>::ref_type DynamicDataFactoryImpl::create_data(
        traits<DynamicType>::ref_type type) noexcept
{
    return std::make_shared<DynamicDataImpl>(type);
}

ReturnCode_t DynamicDataFactoryImpl::delete_data(
        traits<DynamicData>::ref_type& data) noexcept
{
    data.reset();
    return RETCODE_OK;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
