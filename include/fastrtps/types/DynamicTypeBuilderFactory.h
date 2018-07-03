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

#include <fastrtps/types/TypesBase.h>

namespace eprosima{
namespace fastrtps{
namespace types{

class AnnotationDescriptor;
class DynamicTypeBuilder;
class TypeDescriptor;
class TypeObject;
class DynamicType;

class DynamicTypeBuilderFactory
{
public:
    RTPS_DllAPI static DynamicTypeBuilderFactory* get_instance();
    RTPS_DllAPI static ResponseCode DeleteInstance();

    ~DynamicTypeBuilderFactory();

    RTPS_DllAPI DynamicType* get_primitive_type(TypeKind kind);
    RTPS_DllAPI ResponseCode delete_type(DynamicType* type);

    RTPS_DllAPI DynamicTypeBuilder* create_type(const TypeDescriptor* descriptor);
	RTPS_DllAPI DynamicTypeBuilder* create_type_copy(const DynamicType* type);
    RTPS_DllAPI DynamicTypeBuilder* create_int32_type();
    RTPS_DllAPI DynamicTypeBuilder* create_uint32_type();
    RTPS_DllAPI DynamicTypeBuilder* create_int16_type();
    RTPS_DllAPI DynamicTypeBuilder* create_uint16_type();
    RTPS_DllAPI DynamicTypeBuilder* create_int64_type();
    RTPS_DllAPI DynamicTypeBuilder* create_uint64_type();
    RTPS_DllAPI DynamicTypeBuilder* create_float32_type();
    RTPS_DllAPI DynamicTypeBuilder* create_float64_type();
    RTPS_DllAPI DynamicTypeBuilder* create_float128_type();
    RTPS_DllAPI DynamicTypeBuilder* create_char8_type();
    RTPS_DllAPI DynamicTypeBuilder* create_char16_type();
    RTPS_DllAPI DynamicTypeBuilder* create_bool_type();
    RTPS_DllAPI DynamicTypeBuilder* create_byte_type();
    RTPS_DllAPI DynamicTypeBuilder* create_string_type(uint32_t bound);
	RTPS_DllAPI DynamicTypeBuilder* create_wstring_type(uint32_t bound);
	RTPS_DllAPI DynamicTypeBuilder* create_sequence_type(const DynamicType* element_type, uint32_t bound);
	RTPS_DllAPI DynamicTypeBuilder* create_array_type(const DynamicType* element_type, std::vector<uint32_t> bounds);
	RTPS_DllAPI DynamicTypeBuilder* create_map_type(DynamicType* key_element_type, DynamicType* element_type, uint32_t bound);
	RTPS_DllAPI DynamicTypeBuilder* create_bitmask_type(uint32_t bound);
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
