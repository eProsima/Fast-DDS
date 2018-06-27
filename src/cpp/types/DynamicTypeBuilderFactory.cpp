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

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/DynamicType.h>

namespace eprosima {
namespace fastrtps {
namespace types {

DynamicTypeBuilderFactory::DynamicTypeBuilderFactory()
{
}

DynamicTypeBuilderFactory* DynamicTypeBuilderFactory::get_instance()
{
    return nullptr;
}

ResponseCode DynamicTypeBuilderFactory::DeleteInstance()
{
    return ResponseCode::RETCODE_OK;
}

DynamicType* DynamicTypeBuilderFactory::get_primitive_type(TypeKind /*kind*/)
{
    return nullptr;
}
/*
DynamicTypeBuilder DynamicTypeBuilderFactory::create_type(TypeDescriptor descriptor)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_copy(DynamicType type)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_type_object(TypeObject type_object)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_string_type(uint32_t bound)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_wstring_type(uint32_t bound)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_sequence_type(DynamicType element_type, uint32_t bound)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_array_type(DynamicType element_type, std::vector<uint32_t> bound)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_map_type(DynamicType key_element_type, DynamicType element_type, uint32_t bound)
{
}

DynamicTypeBuilder DynamicTypeBuilderFactory::create_bitmask_type(uint32_t bound)
{
}
*/

//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_uri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
//DynamicTypeBuilder DynamicTypeBuilderFactory::create_type_w_document(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);
ResponseCode DynamicTypeBuilderFactory::delete_type(DynamicType type)
{
    return ResponseCode::RETCODE_OK;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
