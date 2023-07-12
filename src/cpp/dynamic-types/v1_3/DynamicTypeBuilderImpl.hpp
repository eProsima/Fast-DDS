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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_IMPL_HPP
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_IMPL_HPP

#include <fastdds/dds/log/Log.hpp>
#include <dynamic-types/v1_3/TypeState.hpp>
#include <fastrtps/types/v1_3/DynamicTypeBuilder.hpp>
#include <fastrtps/utils/custom_allocators.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class MemberDescriptorImpl;
class DynamicTypeImpl;
class DynamicTypeBuilderFactoryImpl;

class DynamicTypeBuilderImpl final
    : public TypeState
    , public eprosima::detail::external_reference_counting<DynamicTypeBuilderImpl>
{
    using builder_allocator = eprosima::detail::BuilderAllocator<DynamicTypeImpl, DynamicTypeBuilderImpl, false>;

    friend builder_allocator;

    static void after_construction(
            DynamicTypeImpl* b);

    static void before_destruction(
            DynamicTypeImpl* b);

    static ReturnCode_t delete_type(
            const DynamicTypeImpl& type) noexcept;

    static const DynamicTypeImpl& create_copy(
            const DynamicTypeImpl& type) noexcept;

    // Only create objects from the associated factory
    struct use_the_create_method
    {
        explicit use_the_create_method() = default;
    };

    DynamicTypeBuilder interface_;

    MemberId current_member_id_{0};

    mutable std::shared_ptr<const DynamicTypeImpl> instance_; //!< Instance of the associated type object

    bool check_union_configuration(
            const MemberDescriptorImpl& descriptor);

    void clear();

    DynamicTypeBuilderImpl(
            const DynamicTypeBuilderImpl&) = default;
    DynamicTypeBuilderImpl(
            DynamicTypeBuilderImpl&&) = delete;
    DynamicTypeBuilderImpl& operator =(
            const DynamicTypeBuilderImpl&) = default;
    DynamicTypeBuilderImpl& operator =(
            DynamicTypeBuilderImpl&&) = delete;

    //! This method only adds an empty element to the members collection with the right index
    member_iterator add_empty_member(
            uint32_t index,
            const std::string& name);

public:

    DynamicTypeBuilderImpl(
            use_the_create_method);

    DynamicTypeBuilderImpl(
            use_the_create_method,
            const DynamicTypeBuilderImpl& builder);

    DynamicTypeBuilderImpl(
            use_the_create_method,
            const TypeState& descriptor,
            bool is_static = false);

    ~DynamicTypeBuilderImpl() = default;

    static const DynamicTypeBuilderImpl& get_implementation(const DynamicTypeBuilder& t)
    {
        return get_implementation(const_cast<DynamicTypeBuilder&>(t));
    }

    static DynamicTypeBuilderImpl& get_implementation(DynamicTypeBuilder& t)
    {
        return *(DynamicTypeBuilderImpl*)((const char*)&t -
                (::size_t)&reinterpret_cast<char const volatile&>((((DynamicTypeBuilderImpl*)0)->interface_)));
    }

    DynamicTypeBuilder& get_interface()
    {
       return interface_;
    }

    const DynamicTypeBuilder& get_interface() const
    {
       return interface_;
    }

    friend class DynamicTypeBuilderFactoryImpl;

    using TypeState::get_annotation;
    using TypeState::get_annotation_count;

    using TypeState::annotation_set_key;
    using TypeState::annotation_set_must_understand;
    using TypeState::annotation_set_mutable;
    using TypeState::annotation_set_final;
    using TypeState::annotation_set_appendable;

    using TypeState::get_descriptor;

    /**
     * Add a new \a member to the underlying @ref DynamicTypeImpl by move
     * @param[in] descriptor r-value to @ref MemberDescriptorImpl
     * @return standard @ref ReturnCode_t
     */
    ReturnCode_t add_member(
            MemberDescriptorImpl&& descriptor) noexcept;

    //! Ancillary template to build inline the @ref MemberDescriptorImpl argument
    template<typename ... Ts>
    ReturnCode_t add_member(
            Ts&&... Args) noexcept
    {
        return add_member(MemberDescriptorImpl{std::forward<Ts>(Args)...});
    }

    using AnnotationManager::apply_annotation;

    /**
     * Apply the given annotation to this member (see [standard] section 7.5.2.9.6)
     * @param[in] id Identifies the member to which the annotation shall be applied
     * @param[in] Args @ref AnnotationDescriptorImpl constructor arguments
     * @return standard @ref ReturnCode_t
     * @remarks the @ref AnnotationDescriptorImpl arguments will be perfect forwarded in order to avoid copies
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
     * Creates an immutable @ref DynamicTypeImpl object containing a snapshot of this
     * builder's current state.
     * @remark Subsequent changes to this builder, if any, shall have no observable effect on the states
     *         of any previously created @ref DynamicTypeImpl objects
     * @remark Once a @ref DynamicTypeImpl object is created it is cached, thus the same object will be returned
     *         until the builder state is modified
     * @attention This class is not thread safe. The only guarantee is that concurrency is safe once avoided
     *            non-const method usage.
     * @return new @ref DynamicTypeImpl object reference
     */
    std::shared_ptr<const DynamicTypeImpl> build() const;

    using TypeState::get_member_count;

    bool is_discriminator_type() const;

    using TypeState::set_name;

    using TypeState::set_base_type;

    using TypeState::get_member_by_name;

    using TypeState::get_all_members_by_name;

    using TypeState::get_all_members_by_id;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_IMPL_HPP
