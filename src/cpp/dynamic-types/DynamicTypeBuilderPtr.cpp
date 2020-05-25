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

#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilder_ptr::DynamicTypeBuilder_ptr(DynamicTypeBuilder* pType)
    : Base(pType, [](DynamicTypeBuilder* pType)
{
    DynamicTypeBuilderFactory::get_instance()->delete_builder(pType);
})
{
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
