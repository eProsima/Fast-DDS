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

#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include "DynamicTypeBuilderFactoryImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

traits<DynamicTypeBuilderFactory>::ref_type DynamicTypeBuilderFactory::get_instance()
{
    return DynamicTypeBuilderFactoryImpl::get_instance();
}

ReturnCode_t DynamicTypeBuilderFactory::delete_instance()
{
    // Delegate into the implementation class
    return DynamicTypeBuilderFactoryImpl::delete_instance();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
