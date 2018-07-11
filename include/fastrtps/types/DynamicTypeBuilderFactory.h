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
class TypeIdentifier;
class MemberDescriptor;
class TypeObject;
class DynamicType;

class DynamicTypeBuilderFactory
{
public:
    RTPS_DllAPI static DynamicTypeBuilderFactory* GetInstance();
    RTPS_DllAPI static ResponseCode DeleteInstance();

    ~DynamicTypeBuilderFactory();

    RTPS_DllAPI DynamicType* GetPrimitiveType(TypeKind kind);
    RTPS_DllAPI ResponseCode DeleteType(DynamicType* type);

    RTPS_DllAPI DynamicType* BuildType(const TypeDescriptor* descriptor);
    RTPS_DllAPI DynamicType* BuildType(const DynamicType* other);
    RTPS_DllAPI DynamicTypeBuilder* CreateCustomType(const TypeDescriptor* descriptor);
	RTPS_DllAPI DynamicTypeBuilder* CreateTypeCopy(const DynamicType* type);
    RTPS_DllAPI DynamicTypeBuilder* CreateInt32Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint32Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateInt16Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint16Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateInt64Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint64Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat32Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat64Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat128Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateChar8Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateChar16Type();
    RTPS_DllAPI DynamicTypeBuilder* CreateBoolType();
    RTPS_DllAPI DynamicTypeBuilder* CreateByteType();
    RTPS_DllAPI DynamicTypeBuilder* CreateStringType(uint32_t bound = MAX_STRING_LENGTH);
	RTPS_DllAPI DynamicTypeBuilder* CreateWstringType(uint32_t bound = MAX_STRING_LENGTH);
	RTPS_DllAPI DynamicTypeBuilder* CreateSequenceType(const DynamicType* element_type, uint32_t bound = MAX_ELEMENTS_COUNT);
	RTPS_DllAPI DynamicTypeBuilder* CreateArrayType(const DynamicType* element_type, std::vector<uint32_t> bounds);
	RTPS_DllAPI DynamicTypeBuilder* CreateMapType(DynamicType* key_element_type, DynamicType* element_type, uint32_t bound = MAX_ELEMENTS_COUNT);
    RTPS_DllAPI DynamicTypeBuilder* CreateBitmaskType(uint32_t bound);
    RTPS_DllAPI DynamicTypeBuilder* CreateBitsetType(uint32_t bound);
    RTPS_DllAPI DynamicTypeBuilder* CreateAliasType(DynamicType* base_type, const std::string& sName);
    RTPS_DllAPI DynamicTypeBuilder* CreateEnumType();
    RTPS_DllAPI DynamicTypeBuilder* CreateStructType();
    RTPS_DllAPI DynamicTypeBuilder* CreateChildStructType(DynamicType* parent_type);
    RTPS_DllAPI DynamicTypeBuilder* CreateUnionType(DynamicType* discriminator_type);
    //DynamicTypeBuilder* CreateTypeWTypeObject(TypeObject* type_object);
    //DynamicTypeBuilder* CreateTypeWUri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
	//DynamicTypeBuilder* CreateTypeWDocument(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);

    RTPS_DllAPI void BuildTypeIdentifier(const TypeDescriptor* descriptor, TypeIdentifier& identifier) const;
    RTPS_DllAPI void BuildTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<MemberDescriptor*>* members = nullptr) const;

    RTPS_DllAPI bool IsEmpty() const;

protected:
	DynamicTypeBuilderFactory();

    inline void AddTypeToList(DynamicType* pType);

    void BuildAliasTypeObject(const TypeDescriptor* descriptor, TypeObject& object) const;
    void BuildEnumTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<MemberDescriptor*> members) const;
    void BuildStructTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<MemberDescriptor*> members) const;
    void BuildUnionTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<MemberDescriptor*> members) const;

#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::vector<DynamicType*> mTypesList;
#endif

};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
