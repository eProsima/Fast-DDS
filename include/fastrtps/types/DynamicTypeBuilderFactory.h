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
#include <fastrtps/types/DynamicTypePtr.h>
#include <mutex>

//#define DISABLE_DYNAMIC_MEMORY_CHECK

namespace eprosima {
namespace fastrtps {
namespace types {

class AnnotationDescriptor;
class DynamicTypeBuilder;
class TypeDescriptor;
class TypeIdentifier;
class MemberDescriptor;
class TypeObject;
class DynamicType;
class DynamicType_ptr;

class DynamicTypeBuilderFactory
{
public:
    RTPS_DllAPI static DynamicTypeBuilderFactory* GetInstance();
    RTPS_DllAPI static ResponseCode DeleteInstance();

    ~DynamicTypeBuilderFactory();

    RTPS_DllAPI DynamicType_ptr GetPrimitiveType(TypeKind kind);
    RTPS_DllAPI ResponseCode DeleteBuilder(DynamicTypeBuilder* builder);
    RTPS_DllAPI ResponseCode DeleteType(DynamicType* type);

    RTPS_DllAPI DynamicTypeBuilder* CreateCustomBuilder(const TypeDescriptor* descriptor, const std::string& name = "");
    RTPS_DllAPI DynamicTypeBuilder* CreateBuilderCopy(const DynamicTypeBuilder* type);
    RTPS_DllAPI DynamicTypeBuilder* CreateInt32Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint32Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateInt16Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint16Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateInt64Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateUint64Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat32Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat64Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateFloat128Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateChar8Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateChar16Builder();
    RTPS_DllAPI DynamicTypeBuilder* CreateBoolBuilder();
    RTPS_DllAPI DynamicTypeBuilder* CreateByteBuilder();
    RTPS_DllAPI DynamicTypeBuilder* CreateStringBuilder(uint32_t bound = MAX_STRING_LENGTH);
    RTPS_DllAPI DynamicTypeBuilder* CreateWstringBuilder(uint32_t bound = MAX_STRING_LENGTH);
    RTPS_DllAPI DynamicTypeBuilder* CreateSequenceBuilder(const DynamicTypeBuilder* element_type, uint32_t bound = MAX_ELEMENTS_COUNT);
    RTPS_DllAPI DynamicTypeBuilder* CreateSequenceBuilder(const DynamicType_ptr type, uint32_t bound = MAX_ELEMENTS_COUNT);
    RTPS_DllAPI DynamicTypeBuilder* CreateArrayBuilder(const DynamicTypeBuilder* element_type, const std::vector<uint32_t>& bounds);
    RTPS_DllAPI DynamicTypeBuilder* CreateArrayBuilder(const DynamicType_ptr type, const std::vector<uint32_t>& bounds);
    RTPS_DllAPI DynamicTypeBuilder* CreateMapBuilder(DynamicTypeBuilder* key_element_type, DynamicTypeBuilder* element_type, uint32_t bound = MAX_ELEMENTS_COUNT);
    RTPS_DllAPI DynamicTypeBuilder* CreateMapBuilder(DynamicType_ptr key_type, DynamicType_ptr value_type, uint32_t bound = MAX_ELEMENTS_COUNT);
    RTPS_DllAPI DynamicTypeBuilder* CreateBitmaskBuilder(uint32_t bound);
    RTPS_DllAPI DynamicTypeBuilder* CreateBitsetBuilder(uint32_t bound);
    RTPS_DllAPI DynamicTypeBuilder* CreateAliasBuilder(DynamicTypeBuilder* base_type, const std::string& sName);
    RTPS_DllAPI DynamicTypeBuilder* CreateAliasBuilder(DynamicType_ptr base_type, const std::string& sName);
    RTPS_DllAPI DynamicTypeBuilder* CreateEnumBuilder();
    RTPS_DllAPI DynamicTypeBuilder* CreateStructBuilder();
    RTPS_DllAPI DynamicTypeBuilder* CreateChildStructBuilder(DynamicTypeBuilder* parent_type);
    RTPS_DllAPI DynamicTypeBuilder* CreateUnionBuilder(DynamicTypeBuilder* discriminator_type);
    RTPS_DllAPI DynamicTypeBuilder* CreateUnionBuilder(DynamicType_ptr discriminator_type);
    //DynamicTypeBuilder* CreateTypeWTypeObject(TypeObject* type_object);
    //DynamicTypeBuilder* CreateTypeWUri(const std::string& document_url, const std::string& type_name, IncludePathSeq include_paths);
    //DynamicTypeBuilder* CreateTypeWDocument(const std::string& document, const std::string& type_name, IncludePathSeq include_paths);

    RTPS_DllAPI DynamicType_ptr CreateAnnotationPrimitive();
    RTPS_DllAPI DynamicType_ptr CreateType(const TypeDescriptor* descriptor, const std::string& name = "");
    RTPS_DllAPI DynamicType_ptr CreateType(const DynamicTypeBuilder* other);
    RTPS_DllAPI DynamicType_ptr CreateAliasType(DynamicTypeBuilder* base_type, const std::string& sName);
    RTPS_DllAPI DynamicType_ptr CreateAliasType(DynamicType_ptr base_type, const std::string& sName);
    RTPS_DllAPI DynamicType_ptr CreateInt32Type();
    RTPS_DllAPI DynamicType_ptr CreateUint32Type();
    RTPS_DllAPI DynamicType_ptr CreateInt16Type();
    RTPS_DllAPI DynamicType_ptr CreateUint16Type();
    RTPS_DllAPI DynamicType_ptr CreateInt64Type();
    RTPS_DllAPI DynamicType_ptr CreateUint64Type();
    RTPS_DllAPI DynamicType_ptr CreateFloat32Type();
    RTPS_DllAPI DynamicType_ptr CreateFloat64Type();
    RTPS_DllAPI DynamicType_ptr CreateFloat128Type();
    RTPS_DllAPI DynamicType_ptr CreateChar8Type();
    RTPS_DllAPI DynamicType_ptr CreateChar16Type();
    RTPS_DllAPI DynamicType_ptr CreateBoolType();
    RTPS_DllAPI DynamicType_ptr CreateByteType();
    RTPS_DllAPI DynamicType_ptr CreateStringType(uint32_t bound = MAX_STRING_LENGTH);
    RTPS_DllAPI DynamicType_ptr CreateWstringType(uint32_t bound = MAX_STRING_LENGTH);
    RTPS_DllAPI DynamicType_ptr CreateBitsetType(uint32_t bound);

    RTPS_DllAPI void BuildTypeIdentifier(const DynamicType_ptr type, TypeIdentifier& identifier,
        bool complete = true) const;
    RTPS_DllAPI void BuildTypeIdentifier(const TypeDescriptor* descriptor, TypeIdentifier& identifier,
        bool complete = true) const;
    RTPS_DllAPI void BuildTypeObject(const DynamicType_ptr type, TypeObject& object,
        bool complete = true) const;
    RTPS_DllAPI void BuildTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*>* members = nullptr,
        bool complete = true) const;

    RTPS_DllAPI bool IsEmpty() const;

protected:
    DynamicTypeBuilderFactory();

    inline void AddBuilderToList(DynamicTypeBuilder* pBuilder);

    DynamicType_ptr BuildType(DynamicType_ptr other);

    void BuildAliasTypeObject(const TypeDescriptor* descriptor, TypeObject& object, bool complete = true) const;
    void BuildEnumTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;
    void BuildStructTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;
    void BuildUnionTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;
    void BuildBitsetTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;
    void BuildBitmaskTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;
    void BuildAnnotationTypeObject(const TypeDescriptor* descriptor, TypeObject& object,
        const std::vector<const MemberDescriptor*> members, bool complete = true) const;

#ifndef DISABLE_DYNAMIC_MEMORY_CHECK
    std::vector<DynamicTypeBuilder*> mBuildersList;
    mutable std::recursive_mutex mMutex;
#endif
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_TYPE_BUILDER_FACTORY_H
