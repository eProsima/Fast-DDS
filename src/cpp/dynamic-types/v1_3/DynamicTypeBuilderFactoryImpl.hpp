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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_IMPL_HPP
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_IMPL_HPP

#include <fastrtps/types/AnnotationParameterValue.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/custom_allocators.hpp>

#include <cassert>
#include <memory>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeIdentifier;
class TypeObject;
class AnnotationParameterValue;

namespace v1_3 {

class TypeState;
class MemberDescriptorImpl;
class DynamicTypeImpl;
class DynamicTypeBuilderImpl;

/**
 * This class is conceived as a singleton charged of creation of @ref DynamicTypeBuilderImpl objects.
 * For simplicity direct primitive types instantiation is also possible.
 */
class DynamicTypeBuilderFactoryImpl final
{
    using builder_allocator = eprosima::detail::BuilderAllocator<DynamicTypeBuilderImpl, DynamicTypeBuilderFactoryImpl, true>;

    // BuilderAllocator ancillary
    builder_allocator& get_allocator()
    {
        // stateful, this factory must outlive all builders
        static builder_allocator alloc{*this};
        return alloc;
    }

    friend builder_allocator;

    //! allocator callback
    void after_construction(
            DynamicTypeBuilderImpl* b);

    //! allocator callback
    void before_destruction(
            DynamicTypeBuilderImpl* b);

    // free any allocated resources
    void reset();

    DynamicTypeBuilderFactoryImpl() = default;

    //! auxiliary method for primitive creation that atomically modifies the dynamic_tracker
    std::shared_ptr<DynamicTypeBuilderImpl> new_primitive_builder(
            TypeKind kind) noexcept;

    //! auxiliary method for string creation that atomically modifies the dynamic_tracker
    std::shared_ptr<DynamicTypeBuilderImpl> new_unlimited_string_builder(
            bool large) noexcept;

    void build_alias_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_string8_type_code(
            const TypeState& descriptor) const;

    void build_string16_type_code(
            const TypeState& descriptor) const;

    void build_sequence_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_array_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_map_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_enum_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_struct_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_union_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_bitset_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_bitmask_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_annotation_type_code(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void set_annotation_default_value(
            AnnotationParameterValue& apv,
            const MemberDescriptorImpl& member) const;

    void apply_type_annotations(
            AppliedAnnotationSeq& annotations,
            const TypeState& descriptor) const;

public:

    ~DynamicTypeBuilderFactoryImpl();

    /**
     * Returns the singleton factory object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @return @ref DynamicTypeBuilderFactoryImpl &
     */
    static DynamicTypeBuilderFactoryImpl& get_instance() noexcept;

    /**
     * Resets the state of the factory
     * @remark This method is thread-safe.
     * @return standard @ref ReturnCode_t
     */
    static ReturnCode_t delete_instance() noexcept;

    /**
     * Create a new @ref DynamicTypeBuilderImpl object based on the given @ref TypeState state.
     * @remark This method is thread-safe.
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.6 this method is
     *         called `create_type` which is misguiding. Note it returns a builder associated with the type.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] td object state to copy
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_type(
            const TypeState& td) noexcept;

    /**
     * Create a new @ref DynamicTypeBuilderImpl object based on the given @ref TypeState object.
     * @remark This method is thread-safe.
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.7 this method is
     *         called `create_type_copy` which is misguiding. Note it returns a builder associated with the type.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] type @ref TypeState object
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_type_copy(
            const TypeState& type) noexcept;

    /**
     * Create a new @ref DynamicTypeImpl object based on the given @ref DynamicTypeImpl object.
     * @remark This method is thread-safe.
     * @remark This method will always create a new object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] type @ref DynamicTypeImpl object
     * @return new @ref DynamicTypeImpl object copy
     */
    const DynamicTypeImpl& create_copy(
            const DynamicTypeImpl& type) noexcept;

    /**
     * Retrieve the cached @ref DynamicTypeImpl object associated to a given primitive
     * @remark This method is thread-safe.
     * @param[in] kind type identifying the primitive type to retrieve
     * @return @ref DynamicTypeImpl object
     */
    std::shared_ptr<const DynamicTypeImpl> get_primitive_type(
            TypeKind kind) noexcept;

    //! alias of `create_type(TypeKind::TK_INT32)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_int32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT32)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_uint32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_INT16)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_int16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT16)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_uint16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_INT64)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_int64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT64)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_uint64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT32)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_float32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT64)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_float64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT128)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_float128_type() noexcept;

    //! alias of `create_type(TypeKind::TK_CHAR8)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_char8_type() noexcept;

    //! alias of `create_type(TypeKind::TK_CHAR16)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_char16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_BOOLEAN)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_bool_type() noexcept;

    //! alias of `create_type(TypeKind::TK_BYTE)`
    std::shared_ptr<const DynamicTypeBuilderImpl> create_byte_type() noexcept;

    //! returns the cache type associated to create_int16_type()
    std::shared_ptr<const DynamicTypeImpl> get_int16_type();

    //! returns the cache type associated to create_uint16_type()
    std::shared_ptr<const DynamicTypeImpl> get_uint16_type();

    //! returns the cache type associated to create_int32_type()
    std::shared_ptr<const DynamicTypeImpl> get_int32_type();

    //! returns the cache type associated to create_uint32_type()
    std::shared_ptr<const DynamicTypeImpl> get_uint32_type();

    //! returns the cache type associated to create_int64_type()
    std::shared_ptr<const DynamicTypeImpl> get_int64_type();

    //! returns the cache type associated to create_uint64_type()
    std::shared_ptr<const DynamicTypeImpl> get_uint64_type();

    //! returns the cache type associated to create_float32_type()
    std::shared_ptr<const DynamicTypeImpl> get_float32_type();

    //! returns the cache type associated to create_float64_type()
    std::shared_ptr<const DynamicTypeImpl> get_float64_type();

    //! returns the cache type associated to create_float128_type()
    std::shared_ptr<const DynamicTypeImpl> get_float128_type();

    //! returns the cache type associated to create_char8_type()
    std::shared_ptr<const DynamicTypeImpl> get_char8_type();

    //! returns the cache type associated to create_char16_type()
    std::shared_ptr<const DynamicTypeImpl> get_char16_type();

    //! returns the cache type associated to create_bool_type()
    std::shared_ptr<const DynamicTypeImpl> get_bool_type();

    //! returns the cache type associated to get_byte_type()
    std::shared_ptr<const DynamicTypeImpl> get_byte_type();

    /**
     * Frees any framework resources associated with the given type according with [standard] section 7.5.2.2.10.
     * @remark This method is thread-safe.
     * @remark RAII will prevent memory leaks even if this method is not called.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicTypeImpl object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    ReturnCode_t delete_type(
            const DynamicTypeImpl& type) noexcept;

    /**
     * Frees any framework resources associated with the given type according with [standard] section 7.5.2.2.10.
     * @remark This method is thread-safe.
     * @remark RAII will prevent memory leaks even if this method is not called.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicTypeBuilderImpl object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    ReturnCode_t delete_type(
            const DynamicTypeBuilderImpl& type) noexcept;

    /**
     * Returns a singleton @ref DynamicTypeBuilderImpl object
     * @tparam kind @ref eprosima::fastrtps::types::TypeKind that identifies the singleton to return
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @remark The singleton cannot be modified. In order to get a modifiable builder use @ref create_type().
     * @return singleton @ref DynamicTypeBuilderImpl object
     */
    template<TypeKind kind>
    typename std::enable_if<is_primitive_t<kind>::value, std::shared_ptr<const DynamicTypeBuilderImpl>>::type
    create_primitive_type() noexcept
    {
        // C++11 compiler uses double-checked locking pattern to avoid concurrency issues
        static std::shared_ptr<const DynamicTypeBuilderImpl> builder = { new_primitive_builder(kind) };
        if (builder)
        {
            builder->add_ref();
        }
        return builder;
    }

    /**
     * Returns a singleton @ref DynamicTypeBuilderImpl object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @remark The singleton cannot be modified. In order to get a modifiable builder use @ref create_type().
     * @param kind @ref eprosima::fastrtps::types::TypeKind that identifies the singleton to return
     * @return singleton @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<const DynamicTypeBuilderImpl> create_primitive_type(
            TypeKind kind) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a bounded string type.
     * @remark The element type of the typed returned is a char8
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_string_type` which is misguiding. Note it returns a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<const DynamicTypeBuilderImpl> create_string_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a bounded wstring type.
     * @remark The element type of the typed returned is a char16
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_wstring_type` which is misguiding. Note it returns a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<const DynamicTypeBuilderImpl> create_wstring_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a sequence
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.4 this method is
     *         called `create_sequence_type` which is misguiding. Note it returns a builder.
     * @param[in] type @ref DynamicTypeImpl which becomes the element type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_sequence_type(
            const DynamicTypeImpl& type,
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing an array
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.1 this method is
     *         called `create_array_type` which is misguiding. Note the return value is a builder.
     * @param[in] type @ref DynamicTypeImpl which becomes the element type
     * @param[in] bounds `uint32_t` representing the desired dimensions
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_array_type(
            const DynamicTypeImpl& type,
            const std::vector<uint32_t>& bounds) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a map
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.3 this method is
     *         called `create_map_type` which is misguiding. Note the return value is a builder.
     * @param[in] key_type @ref DynamicTypeImpl which becomes the map's key type
     * @param[in] value_type @ref DynamicTypeImpl which becomes the map's value type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_map_type(
            const DynamicTypeImpl& key_type,
            const DynamicTypeImpl& value_type,
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a bitmask
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.2 this method is
     *         called `create_bitmask_type` which is misguiding. Note the return value is a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_bitmask_type(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_bitset_type(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing an alias
     * @param[in] base_type @ref DynamicTypeImpl to be referenced
     * @param[in] sName new alias name
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_alias_type(
            const DynamicTypeImpl& base_type,
            const std::string& sName);

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing an enum
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_enum_type();

    /**
     * Returns a @ref DynamicTypeBuilderImpl associated with a `TypeKind::TK_STRUCTURE`
     * @return @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_struct_type() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a subclass
     * @param[in] parent_type @ref DynamicTypeImpl identifying the desired superclass
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_child_struct_type(
            const DynamicTypeImpl& parent_type);

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a union
     * @param[in] discriminator_type @ref DynamicTypeImpl associated to the union's discriminator
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<DynamicTypeBuilderImpl> create_union_type(
            const DynamicTypeImpl& discriminator_type);

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing an annotation
     * @param[in] name string annotation identifier
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<const DynamicTypeImpl> create_annotation_primitive(
            const std::string& name);

    //! returns type instantiation of the @ref DynamicTypeBuilderFactoryImpl::create_alias_type builder
    std::shared_ptr<const DynamicTypeImpl> get_alias_type(
            const DynamicTypeImpl& base_type,
            const std::string& sName);

    //! returns the cache type associated to create_string_type()
    std::shared_ptr<const DynamicTypeImpl> get_string_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    //! returns the cache type associated to create_wstring_type()
    std::shared_ptr<const DynamicTypeImpl> get_wstring_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderImpl object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilderImpl object
     */
    std::shared_ptr<const DynamicTypeImpl> get_bitset_type(
            uint32_t bound);

    void build_type_identifier(
            const TypeState& descriptor,
            TypeIdentifier& identifier,
            bool complete = true) const;

    void build_type_object(
            const TypeState& descriptor,
            TypeObject& object,
            bool complete = true,
            bool force = false) const;

    bool is_empty() const;

    //! ostream ancillary
    static const int indentation_index; //!< indentation
    static const int object_index; //!< previous object in << stack
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_IMPL_HPP
