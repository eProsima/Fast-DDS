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

#ifndef TYPE_OBJECT_TYPE_FACTORY_H
#define TYPE_OBJECT_TYPE_FACTORY_H

#include <fastrtps/types/TypeObject.h>
#include <fastrtps/types/DynamicType.h>
#include <mutex>

namespace eprosima{
namespace fastrtps{
namespace types{

class TypeObjectFactory
{
public:
    RTPS_DllAPI static TypeObjectFactory* GetInstance();
    RTPS_DllAPI static ResponseCode DeleteInstance();

    ~TypeObjectFactory();

    RTPS_DllAPI const TypeObject* GetTypeObject(const std::string &type_name) const;
    RTPS_DllAPI const TypeObject* GetTypeObject(const TypeIdentifier* identifier) const;

    RTPS_DllAPI TypeKind GetTypeKind(const std::string &type_name) const;
    RTPS_DllAPI std::string GetTypeName(const TypeKind kind) const;

    RTPS_DllAPI const TypeIdentifier* GetPrimitiveTypeIdentifier(TypeKind kind);
    //RTPS_DllAPI TypeIdentifier* TryCreateTypeIdentifier(const std::string &type_name);
    RTPS_DllAPI const TypeIdentifier* GetTypeIdentifier(const std::string &type_name) const;
    RTPS_DllAPI const TypeIdentifier* GetStringIdentifier(uint32_t bound, bool wide = false);
    RTPS_DllAPI const TypeIdentifier* GetSequenceIdentifier(const std::string &type_name, uint32_t bound);
    RTPS_DllAPI const TypeIdentifier* GetArrayIdentifier(const std::string &type_name, const std::vector<uint32_t> &bound);
    RTPS_DllAPI const TypeIdentifier* GetMapIdentifier(const std::string &key_type_name,
        const std::string &value_type_name, uint32_t bound);

    RTPS_DllAPI DynamicType* BuildDynamicType(const std::string& name, const TypeIdentifier* identifier,
        const TypeObject* object = nullptr) const;

    RTPS_DllAPI void AddTypeIdentifier(const std::string &type_name, const TypeIdentifier* identifier);
    RTPS_DllAPI void AddTypeObject(const std::string &type_name, const TypeIdentifier* identifier,
        const TypeObject* object);
protected:
	TypeObjectFactory();
    std::map<const std::string, const TypeIdentifier*> m_Identifiers;
    std::map<const TypeIdentifier*, const TypeObject*> m_Objects;


    std::string getStringTypeName(uint32_t bound, bool wide, bool generate_identifier = true);
    std::string getSequenceTypeName(const std::string &type_name, uint32_t bound, bool generate_identifier = true);
    std::string getArrayTypeName(const std::string &type_name, const std::vector<uint32_t> &bound,
        bool generate_identifier = true);
    std::string getArrayTypeName(const std::string &type_name, const std::vector<uint32_t> &bound, uint32_t &ret_size,
        bool generate_identifier = true);
    std::string getMapTypeName(const std::string &key_type_name, const std::string &value_type_name, uint32_t bound,
        bool generate_identifier = true);
    //TODO: TypeDescriptor* BuildTypeDescriptorFromObject(TypeDescriptor* descriptor, const TypeObject* object) const;
    //TODO: void BuildTypeDescriptorFromMinimalObject(TypeDescriptor* descriptor, const MinimalTypeObject &minimal) const;

    DynamicType* BuildDynamicType(const TypeIdentifier* identifier, const TypeObject* object) const;

    //MemberDescriptor* BuildMemberDescriptor(const TypeIdentifier* identifier, const TypeObject* object = nullptr);
private:
    mutable std::mutex m_MutexIdentifiers;
    mutable std::mutex m_MutexObjects;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPE_OBJECT_TYPE_FACTORY_H
