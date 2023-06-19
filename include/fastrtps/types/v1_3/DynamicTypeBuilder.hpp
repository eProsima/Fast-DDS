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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_HPP
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_HPP

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/v1_3/DynamicTypeBuilderPtr.hpp>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>
#include <fastrtps/utils/custom_allocators.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class AnnotationDescriptor;
class MemberDescriptor;
class DynamicType;
class DynamicTypeMember;
class DynamicTypeBuilderFactory;

class DynamicTypeBuilder final
    : public TypeDescriptor
    , public eprosima::detail::external_reference_counting<DynamicTypeBuilder>
{
    using builder_allocator = eprosima::detail::BuilderAllocator<DynamicType, DynamicTypeBuilder, false>;

    friend builder_allocator;

    static void after_construction(
            DynamicType* b);

    static void before_destruction(
            DynamicType* b);

    static ReturnCode_t delete_type(
            const DynamicType* type) noexcept;

    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

    friend std::function<void(const DynamicTypeBuilder*)> dynamic_object_deleter(const DynamicTypeBuilder*);

    MemberId current_member_id_{0};

    mutable DynamicType_ptr instance_; //!< Instance of the associated type object

    bool check_union_configuration(
            const MemberDescriptor& descriptor);

    void clear();

    DynamicTypeBuilder(
            const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder(
            DynamicTypeBuilder&&) = delete;
    DynamicTypeBuilder& operator =(
            const DynamicTypeBuilder&) = default;
    DynamicTypeBuilder& operator =(
            DynamicTypeBuilder&&) = delete;

    //! This method only adds an empty element to the members collection with the right index
    member_iterator add_empty_member(
            uint32_t index,
            const std::string& name);

public:

    DynamicTypeBuilder(
            use_the_create_method);

    DynamicTypeBuilder(
            use_the_create_method,
            const DynamicTypeBuilder& builder);

    DynamicTypeBuilder(
            use_the_create_method,
            const TypeDescriptor& descriptor,
            bool is_static = false);

    ~DynamicTypeBuilder() = default;

    friend class DynamicTypeBuilderFactory;

    using TypeDescriptor::equals;

    /**
     * Underlying state comparison
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] other @ref DynamicType object to compare to
     * @return \b bool `true` on equality
     */
    RTPS_DllAPI bool equals(
            const DynamicType& other) const;

    using TypeDescriptor::get_annotation;
    using TypeDescriptor::get_annotation_count;

    using TypeDescriptor::annotation_set_key;
    using TypeDescriptor::annotation_set_must_understand;
    using TypeDescriptor::annotation_set_mutable;
    using TypeDescriptor::annotation_set_final;
    using TypeDescriptor::annotation_set_appendable;

    using TypeDescriptor::get_descriptor;

    /**
     * Add a new \a member to the underlying @ref DynamicType by move
     * @param[in] descriptor r-value to @ref MemberDescriptor
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI ReturnCode_t add_member(
            MemberDescriptor&& descriptor) noexcept;

    //! Ancillary template to build inline the @ref MemberDescriptor argument
    template<typename ... Ts>
    ReturnCode_t add_member(
            Ts&&... Args) noexcept
    {
        return add_member(MemberDescriptor(std::forward<Ts>(Args)...));
    }

    using AnnotationManager::apply_annotation;

    /**
     * Apply the given annotation to this member (see [standard] section 7.5.2.9.6)
     * @param[in] id Identifies the member to which the annotation shall be applied
     * @param[in] Args @ref AnnotationDescriptor constructor arguments
     * @return standard @ref ReturnCode_t
     * @remarks the @ref AnnotationDescriptor arguments will be perfect forwarded in order to avoid copies
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    template<typename ... Ts>
    ReturnCode_t apply_annotation_to_member(
            MemberId id,
            Ts&&... Args)
    {
        auto it = member_by_id_.find(id);
        if (it != member_by_id_.end())
        {
            it->second->apply_annotation(std::forward<Ts>(Args)...);
            return ReturnCode_t::RETCODE_OK;
        }
        else
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES, "Error applying annotation to member. MemberId not found.");
            return ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
    }

    /**
     * Creates an immutable @ref DynamicType object containing a snapshot of this
     * builder's current state.
     * @remark Subsequent changes to this builder, if any, shall have no observable effect on the states
     *         of any previously created @ref DynamicType objects
     * @remark Once a @ref DynamicType object is created it is cached, thus the same object will be returned
     *         until the builder state is modified
     * @attention This class is not thread safe. The only guarantee is that concurrency is safe once avoided
     *            non-const method usage.
     * @return new @ref DynamicType object reference
     */
    RTPS_DllAPI const DynamicType* build() const;

    RTPS_DllAPI ReturnCode_t copy_from(
            const DynamicTypeBuilder* other);

    using TypeDescriptor::get_member_count;

    bool is_discriminator_type() const;

    using TypeDescriptor::set_name;

    using TypeDescriptor::set_base_type;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_HPP
