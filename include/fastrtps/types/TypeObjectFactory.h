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
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <mutex>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeObjectFactory
{
private:
    mutable std::recursive_mutex m_MutexIdentifiers;
    mutable std::recursive_mutex m_MutexObjects;

protected:
    TypeObjectFactory();
    std::map<const std::string, const TypeIdentifier*> identifiers_; // Basic, builtin and EK_MINIMAL
    std::map<const std::string, const TypeIdentifier*> complete_identifiers_; // Only EK_COMPLETE
    std::map<const TypeIdentifier*, const TypeObject*> objects_; // EK_MINIMAL
    std::map<const TypeIdentifier*, const TypeObject*> complete_objects_; // EK_COMPLETE
    std::map<std::string, std::string> aliases_; // Aliases

    DynamicType_ptr build_dynamic_type(
            TypeDescriptor& descriptor,
            const TypeObject* object,
            const DynamicType_ptr annotation_member_type = nullptr) const;

    const TypeIdentifier* try_get_complete(const TypeIdentifier* identifier) const;

    const TypeIdentifier* get_stored_type_identifier(const TypeIdentifier* identifier) const;

    void nullify_all_entries(const TypeIdentifier* identifier);

    void create_builtin_annotations();

    void apply_type_annotations(
            DynamicTypeBuilder_ptr& type_builder,
            const AppliedAnnotationSeq& annotations) const;

    void apply_member_annotations(
            DynamicTypeBuilder_ptr& parent_type_builder,
            MemberId member_id,
            const AppliedAnnotationSeq& annotations) const;

    std::string get_key_from_hash(
            const DynamicType_ptr annotation_descriptor_type,
            const NameHash& hash) const;

public:
    RTPS_DllAPI static TypeObjectFactory* get_instance();

    RTPS_DllAPI static ResponseCode delete_instance();

    ~TypeObjectFactory();

    RTPS_DllAPI const TypeObject* get_type_object(
            const std::string& type_name,
            bool complete = false) const;

    RTPS_DllAPI const TypeObject* get_type_object(const TypeIdentifier* identifier) const;

    RTPS_DllAPI TypeKind get_type_kind(const std::string& type_name) const;

    RTPS_DllAPI std::string get_type_name(const TypeKind kind) const;

    RTPS_DllAPI std::string get_type_name(const TypeIdentifier* identifier) const;

    RTPS_DllAPI const TypeIdentifier* get_primitive_type_identifier(TypeKind kind) const;

    RTPS_DllAPI const TypeIdentifier* get_type_identifier(
            const std::string& type_name,
            bool complete = false) const;

    RTPS_DllAPI const TypeIdentifier* get_type_identifier_trying_complete(const std::string& type_name) const;

    RTPS_DllAPI const TypeIdentifier* get_string_identifier(
            uint32_t bound,
            bool wide = false);

    RTPS_DllAPI const TypeIdentifier* get_sequence_identifier(
            const std::string& type_name,
            uint32_t bound,
            bool complete = false);

    RTPS_DllAPI const TypeIdentifier* get_array_identifier(
            const std::string& type_name,
            const std::vector<uint32_t> &bound,
            bool complete = false);

    RTPS_DllAPI const TypeIdentifier* get_map_identifier(
            const std::string& key_type_name,
            const std::string& value_type_name,
            uint32_t bound,
            bool complete = false);

    RTPS_DllAPI DynamicType_ptr build_dynamic_type(
            const std::string& name,
            const TypeIdentifier* identifier,
            const TypeObject* object = nullptr) const;

    RTPS_DllAPI void add_type_identifier(
            const std::string& type_name,
            const TypeIdentifier* identifier);

    RTPS_DllAPI void add_type_object(
            const std::string& type_name,
            const TypeIdentifier* identifier,
            const TypeObject* object);

    RTPS_DllAPI inline void add_alias(
            const std::string& alias_name,
            const std::string& target_type)
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
        aliases_.emplace(std::pair<std::string, std::string>(alias_name, target_type));
    }
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPE_OBJECT_TYPE_FACTORY_H
