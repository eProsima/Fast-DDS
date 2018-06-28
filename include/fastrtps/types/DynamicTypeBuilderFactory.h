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

#ifndef TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
#define TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H

#include <fastrtps/types/MemberId.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor;
class DynamicTypeBuilder;
class TypeDescriptor;
class TypeObject;

class DynamicTypeBuilderFactory
{
public:
	static DynamicTypeBuilderFactory* get_instance();
	static ResponseCode DeleteInstance();

    ~DynamicTypeBuilderFactory();

	DynamicType* get_primitive_type(TypeKind kind);
    ResponseCode delete_type(DynamicType* type);

    DynamicTypeBuilder* create_type(const TypeDescriptor* descriptor);
	DynamicTypeBuilder* create_type_copy(const DynamicType* type);
	DynamicTypeBuilder* create_string_type(uint32_t bound);
	DynamicTypeBuilder* create_wstring_type(uint32_t bound);
	DynamicTypeBuilder* create_sequence_type(const DynamicType* element_type, uint32_t bound);
	DynamicTypeBuilder* create_array_type(const DynamicType* element_type, std::vector<uint32_t> bounds);
	DynamicTypeBuilder* create_map_type(DynamicType* key_element_type, DynamicType* element_type, uint32_t bound);
	DynamicTypeBuilder* create_bitmask_type(uint32_t bound);
    //DynamicTypeBuilder* create_type_w_type_object(TypeObject* type_object);
    //DynamicTypeBuilder* create_type_w_uri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
	//DynamicTypeBuilder* create_type_w_document(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);

protected:
	DynamicTypeBuilderFactory();

    std::vector<DynamicType*> mTypesList;

};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
