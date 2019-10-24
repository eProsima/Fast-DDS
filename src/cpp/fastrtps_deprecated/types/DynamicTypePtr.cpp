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

#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicType_ptr::DynamicType_ptr(
        DynamicType* pType)
    : Base(pType, [](DynamicType* pType)
{
    DynamicTypeBuilderFactory::get_instance()->delete_type(pType);
})
{
}

DynamicType_ptr& DynamicType_ptr::operator =(
        DynamicType* ptr)
{
    return operator =(DynamicType_ptr(ptr));
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
