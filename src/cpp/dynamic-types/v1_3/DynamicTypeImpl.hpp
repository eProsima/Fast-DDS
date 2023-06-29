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

#ifndef TYPES_1_3_DYNAMIC_TYPE_IMPL_HPP
#define TYPES_1_3_DYNAMIC_TYPE_IMPL_HPP

#include <dynamic-types/v1_3/TypeState.hpp>
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

class AnnotationDescriptorImpl;
class DynamicData;
class DynamicTypeMemberImpl;
class DynamicTypeBuilderImpl;
class DynamicTypeBuilderFactoryImpl;

class DynamicTypeImpl final
    : public TypeState
    , public eprosima::detail::external_reference_counting<DynamicTypeImpl>
{
    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

public:

    DynamicTypeImpl(
            use_the_create_method);

    DynamicTypeImpl(
            use_the_create_method,
            const TypeDescriptor& descriptor);

    DynamicTypeImpl(
            use_the_create_method,
            TypeDescriptor&& descriptor);

    ~DynamicTypeImpl();

protected:

    friend class DynamicTypeBuilderImpl;
    friend class types::DynamicDataHelper;

    void clear();

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

    //! check if the type is complex
    bool is_complex_kind() const;

    //! check if the type can be used as a discriminator
    bool is_discriminator_type() const;

    //! returns footprint size if the underlying type is primitive
    size_t get_size() const;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_IMPL_HPP
