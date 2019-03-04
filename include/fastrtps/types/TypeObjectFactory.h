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
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeObjectFactory
{
public:
    RTPS_DllAPI static TypeObjectFactory* GetInstance();
    RTPS_DllAPI static ResponseCode DeleteInstance();

    ~TypeObjectFactory();

    RTPS_DllAPI const TypeObject* GetTypeObject(const std::string &type_name, bool complete = false) const;
    RTPS_DllAPI const TypeObject* GetTypeObject(const TypeIdentifier* identifier) const;

    RTPS_DllAPI TypeKind GetTypeKind(const std::string &type_name) const;
    RTPS_DllAPI std::string GetTypeName(const TypeKind kind) const;
    RTPS_DllAPI std::string GetTypeName(const TypeIdentifier* identifier) const;

    RTPS_DllAPI const TypeIdentifier* GetPrimitiveTypeIdentifier(TypeKind kind) const;
    //RTPS_DllAPI TypeIdentifier* TryCreateTypeIdentifier(const std::string &type_name);
    RTPS_DllAPI const TypeIdentifier* GetTypeIdentifier(const std::string &type_name, bool complete = false) const;
    RTPS_DllAPI const TypeIdentifier* GetTypeIdentifierTryingComplete(const std::string &type_name) const;
    RTPS_DllAPI const TypeIdentifier* GetStringIdentifier(uint32_t bound, bool wide = false);
    RTPS_DllAPI const TypeIdentifier* GetSequenceIdentifier(const std::string &type_name, uint32_t bound,
        bool complete = false);
    RTPS_DllAPI const TypeIdentifier* GetArrayIdentifier(const std::string &type_name,
        const std::vector<uint32_t> &bound, bool complete = false);
    RTPS_DllAPI const TypeIdentifier* GetMapIdentifier(const std::string &key_type_name,
        const std::string &value_type_name, uint32_t bound, bool complete = false);
    // TODO Add bitset and bitmask
    //RTPS_DllAPI const TypeIdentifier* GetBitsetIdentifier(const std::string &type_name,
    //    const std::vector<uint32_t> &bound, bool complete = false);
    //RTPS_DllAPI const TypeIdentifier* GetBitmaskIdentifier(const std::string &type_name,
    //    const std::vector<uint32_t> &bound, bool complete = false);

    RTPS_DllAPI DynamicType_ptr BuildDynamicType(const std::string& name, const TypeIdentifier* identifier,
        const TypeObject* object = nullptr) const;

    RTPS_DllAPI void AddTypeIdentifier(const std::string &type_name, const TypeIdentifier* identifier);
    RTPS_DllAPI void AddTypeObject(const std::string &type_name, const TypeIdentifier* identifier,
        const TypeObject* object);
    RTPS_DllAPI inline void AddAlias(const std::string &alias_name, const std::string &target_type)
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
        m_Aliases.emplace(std::pair<std::string, std::string>(alias_name, target_type));
    }

protected:
    TypeObjectFactory();
    std::map<const std::string, const TypeIdentifier*> m_Identifiers; // Basic, builtin and EK_MINIMAL
    std::map<const std::string, const TypeIdentifier*> m_CompleteIdentifiers; // Only EK_COMPLETE
    std::map<const TypeIdentifier*, const TypeObject*> m_Objects; // EK_MINIMAL
    std::map<const TypeIdentifier*, const TypeObject*> m_CompleteObjects; // EK_COMPLETE
    std::map<std::string, std::string> m_Aliases; // Aliases

    DynamicType_ptr BuildDynamicType(TypeDescriptor &descriptor, const TypeObject* object) const;
    const TypeIdentifier* TryGetComplete(const TypeIdentifier* identifier) const;
    const TypeIdentifier* GetStoredTypeIdentifier(const TypeIdentifier *identifier) const;
    void nullifyAllEntries(const TypeIdentifier *identifier);
private:
    mutable std::recursive_mutex m_MutexIdentifiers;
    mutable std::recursive_mutex m_MutexObjects;
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPE_OBJECT_TYPE_FACTORY_H
