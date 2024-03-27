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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAFACTORYIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAFACTORYIMPL_HPP

#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicDataFactoryImpl : public traits<DynamicDataFactory>::base_type
{
public:

    static traits<DynamicDataFactory>::ref_type get_instance() noexcept;

    static ReturnCode_t delete_instance() noexcept;

    traits<DynamicData>::ref_type create_data(
            traits<DynamicType>::ref_type type) noexcept override;

    ReturnCode_t delete_data(
            traits<DynamicData>::ref_type& data) noexcept override;

    virtual ~DynamicDataFactoryImpl() = default;

private:

    static traits<DynamicDataFactoryImpl>::ref_type instance_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICDATAFACTORYIMPL_HPP
