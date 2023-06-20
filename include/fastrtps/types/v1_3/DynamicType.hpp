// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_1_3_DYNAMIC_TYPE_HPP
#define TYPES_1_3_DYNAMIC_TYPE_HPP

#include <fastrtps/types/v1_3/DynamicTypePtr.hpp>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>
#include <fastrtps/utils/custom_allocators.hpp>

namespace eprosima {

namespace fastdds {
namespace dds {
class DomainParticipantImpl;
} // namespace dds
} // namespace fastdds

namespace fastrtps {
namespace types {
namespace v1_3 {

class AnnotationDescriptor;
class DynamicData;
class DynamicTypeMember;
class DynamicTypeBuilder;
class DynamicTypeBuilderFactory;

class DynamicType final
    : public TypeDescriptor
    , public eprosima::detail::external_reference_counting<DynamicType>
{
    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

    friend void (*dynamic_object_deleter(const DynamicType* ))(const DynamicType*);

public:

    DynamicType(
            use_the_create_method);

    DynamicType(
            use_the_create_method,
            const TypeDescriptor& descriptor);

    DynamicType(
            use_the_create_method,
            TypeDescriptor&& descriptor);

    RTPS_DllAPI ~DynamicType();

    using TypeDescriptor::get_descriptor;

protected:

    friend class DynamicTypeBuilder;
    friend class types::DynamicDataHelper;

    RTPS_DllAPI void clear();

    // Serialization ancillary
    void serialize_empty_data(
            eprosima::fastcdr::Cdr& cdr) const;

    bool deserialize_discriminator(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    void serialize_discriminator(
            const DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

public:

    //! TODO:BARRO move to protected on serialization refactor
    void serializeKey(
            const DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    // Serializes and deserializes the Dynamic Data.
    void serialize(
            const DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    bool deserialize(
            DynamicData& data,
            eprosima::fastcdr::Cdr& cdr) const;

    size_t getCdrSerializedSize(
            const DynamicData& data,
            size_t current_alignment = 0) const;

    size_t getEmptyCdrSerializedSize(
            size_t current_alignment = 0) const;

    size_t getKeyMaxCdrSerializedSize(
            size_t current_alignment = 0) const;

    size_t getMaxCdrSerializedSize(
            size_t current_alignment = 0) const;

public:

    /**
     * State comparison
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] other @ref DynamicType object whose state to compare to
     * @return \b bool `true` on equality
     */
    RTPS_DllAPI bool equals(
            const DynamicType& other) const;

    //! check if the type is complex
    RTPS_DllAPI bool is_complex_kind() const;

    //! check if the type can be used as a discriminator
    RTPS_DllAPI bool is_discriminator_type() const;

    //! returns footprint size if the underlying type is primitive
    RTPS_DllAPI size_t get_size() const;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_HPP
