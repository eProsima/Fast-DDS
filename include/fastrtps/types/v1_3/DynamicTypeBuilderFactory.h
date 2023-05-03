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

#ifndef TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_H
#define TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_H

#include <fastrtps/types/AnnotationParameterValue.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/custom_allocators.hpp>

#include <cassert>
#include <memory>
#include <mutex>
#include <set>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeIdentifier;
class TypeObject;
class AnnotationParameterValue;

namespace v1_3 {

class DynamicTypeBuilderFactory;
class DynamicTypeBuilder;
class DynamicType;

enum class type_tracking
{
    none = 0,
    partial,
    complete
};

namespace detail {

template<type_tracking mode>
struct dynamic_tracker_data;

template<>
struct dynamic_tracker_data<type_tracking::none> {};

template<>
struct dynamic_tracker_data<type_tracking::partial>
{
    std::set<const DynamicTypeBuilder*> builders_list_; /*!< Collection of active DynamicTypeBuilder instances */
    std::set<const DynamicType*> types_list_; /*!< Collection of active DynamicType instances */
    std::mutex mutex_; /*!< atomic access to the collections */
};

template<>
struct dynamic_tracker_data<type_tracking::complete>
    : public dynamic_tracker_data<type_tracking::partial>
{
    std::set<const DynamicTypeBuilder*> primitive_builders_list_; /*!< Collection of static builder instances */
    std::set<const DynamicType*> primitive_types_list_; /*!< Collection of static type instances */
};

} // namespace detail

/**
 * Interface use to track dynamic objects lifetime
 */
template<type_tracking mode>
class dynamic_tracker
    : protected detail::dynamic_tracker_data<mode>
{
    friend class DynamicTypeBuilder;
    friend class DynamicTypeBuilderFactory;

    //! clear collection contents
    void reset() noexcept;
    //! check if there are leakages
    bool is_empty() noexcept;
    //! add primitive builder
    void add_primitive(
            const DynamicTypeBuilder*) noexcept;
    //! add primitive types
    void add_primitive(
            const DynamicType*) noexcept;
    //! add new builder
    bool add(
            const DynamicTypeBuilder*) noexcept;
    //! remove builder
    bool remove(
            const DynamicTypeBuilder*) noexcept;
    //! add new type
    bool add(
            const DynamicType*) noexcept;
    //! remove type
    bool remove(
            const DynamicType*) noexcept;

    // singleton creation
    static dynamic_tracker<mode>& get_dynamic_tracker()
    {
        static dynamic_tracker<mode> dynamic_tracker;
        return dynamic_tracker;
    }

};

#if defined(ENABLE_DYNAMIC_MEMORY_CHECK) \
    && (!defined(_MSC_VER) || _MSC_VER >= 1921) \
    && (!defined(__APPLE__) || _LIBCPP_STD_VER >= 20)
constexpr type_tracking selected_mode = type_tracking::complete;
#else
constexpr type_tracking selected_mode = type_tracking::none;
#endif // if defined(ENABLE_DYNAMIC_MEMORY_CHECK) && (!defined(_MSC_VER) || _MSC_VER >= 1921) && (!defined(__APPLE__) || _LIBCPP_STD_VER >= 20)

class TypeDescriptor;
class MemberDescriptor;

/**
 * This class is conceived as a singleton charged of creation of @ref DynamicTypeBuilder objects.
 * For simplicity direct primitive types instantiation is also possible.
 */
class DynamicTypeBuilderFactory final
{
    using builder_allocator = eprosima::detail::BuilderAllocator<DynamicTypeBuilder, DynamicTypeBuilderFactory, true>;

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
            DynamicTypeBuilder* b);

    //! allocator callback
    void before_destruction(
            DynamicTypeBuilder* b);

    // free any allocated resources
    void reset();

    DynamicTypeBuilderFactory() = default;

    //! auxiliary method for primitive creation that atomically modifies the dynamic_tracker
    RTPS_DllAPI DynamicTypeBuilder_ptr new_primitive_builder(
            TypeKind kind) noexcept;

    //! auxiliary method for string creation that atomically modifies the dynamic_tracker
    DynamicTypeBuilder_ptr new_unlimited_string_builder(
            bool large) noexcept;

    void build_alias_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_string8_type_code(
            const TypeDescriptor& descriptor) const;

    void build_string16_type_code(
            const TypeDescriptor& descriptor) const;

    void build_sequence_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_array_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_map_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_enum_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_struct_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_union_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_bitset_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_bitmask_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void build_annotation_type_code(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true) const;

    void set_annotation_default_value(
            AnnotationParameterValue& apv,
            const MemberDescriptor& member) const;

    void apply_type_annotations(
            AppliedAnnotationSeq& annotations,
            const TypeDescriptor& descriptor) const;

public:

    ~DynamicTypeBuilderFactory();

    /**
     * Returns the singleton factory object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @return @ref DynamicTypeBuilderFactory &
     */
    RTPS_DllAPI static DynamicTypeBuilderFactory& get_instance() noexcept;

    /**
     * Resets the state of the factory
     * @remark This method is thread-safe.
     * @remark On \b DEBUG builds or if explicitly specified using preprocessor macro \b ENABLE_DYNAMIC_MEMORY_CHECK
     *         the factory will track object allocation/deallocation. This method will reset this tracking.
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI static ReturnCode_t delete_instance() noexcept;

    /**
     * Create a new @ref DynamicTypeBuilder object based on the given @ref TypeDescriptor state.
     * @remark This method is thread-safe.
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.6 this method is
     *         called `create_type` which is misguiding. Note it returns a builder associated with the type.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `create_xxxx_builder()` methods.
     * @param[in] td object state to copy
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_type(
            const TypeDescriptor& td) noexcept;

    /**
     * Create a new @ref DynamicTypeBuilder object based on the given @ref DynamicType object.
     * @remark This method is thread-safe.
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.7 this method is
     *         called `create_type_copy` which is misguiding. Note it returns a builder associated with the type.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `create_xxxx_builder()` methods.
     * @param[in] type @ref DynamicType object
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_type_copy(
            const DynamicType& type) noexcept;

    /**
     * Retrieve the cached @ref DynamicType object associated to a given primitive
     * @remark This method is thread-safe.
     * @param[in] kind type identifying the primitive type to retrieve
     * @return @ref DynamicType object
     */
    RTPS_DllAPI DynamicType_ptr get_primitive_type(
            TypeKind kind) noexcept;

    /**
     * Frees any framework resources associated with the given type according with [standard] section 7.5.2.2.10.
     * @remark This method is thread-safe.
     * @remark RAII will prevent memory leaks even if this method is not called.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicType object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI ReturnCode_t delete_type(
            const DynamicType& type) noexcept;

    /**
     * Returns a singleton @ref DynamicTypeBuilder object
     * @tparam kind @ref eprosima::fastrtps::types::TypeKind that identifies the singleton to return
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @remark The singleton cannot be modified. In order to get a modifiable builder use @ref create_type().
     * @return singleton @ref DynamicTypeBuilder object
     */
    template<TypeKind kind>
    typename std::enable_if<is_primitive_t<kind>::value, DynamicTypeBuilder_cptr&>::type
    create_primitive_builder() noexcept
    {
        // C++11 compiler uses double-checked locking pattern to avoid concurrency issues
        static DynamicTypeBuilder_cptr builder = { new_primitive_builder(kind) };
        return builder;
    }

    /**
     * Returns a singleton @ref DynamicTypeBuilder object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @remark The singleton cannot be modified. In order to get a modifiable builder use @ref create_type().
     * @param kind @ref eprosima::fastrtps::types::TypeKind that identifies the singleton to return
     * @return singleton @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_primitive_builder(
            TypeKind kind) noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_INT32>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int32_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_UINT32>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint32_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_INT16>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int16_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_UINT16>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint16_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_INT64>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_int64_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_UINT64>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_uint64_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_FLOAT32>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float32_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_FLOAT64>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float64_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_FLOAT128>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_float128_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_CHAR8>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_char8_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_CHAR16>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_char16_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_BOOLEAN>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_bool_builder() noexcept;

    //! alias of `create_primitive_builder<TypeKind::TK_BYTE>()`
    RTPS_DllAPI DynamicTypeBuilder_cptr& create_byte_builder() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an unbounded string type.
     * @remark The element type of the typed returned is a char8
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_string_type` which is misguiding. It was renamed to simplify interface usage.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_cptr create_string_builder() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bounded string type.
     * @remark The element type of the typed returned is a char8
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_string_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_string_builder(
            uint32_t bound) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an unbounded wstring type.
     * @remark The element type of the typed returned is a char16
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_wstring_type` which is misguiding. It was renamed to simplify interface usage.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_cptr create_wstring_builder() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bounded wstring type.
     * @remark The element type of the typed returned is a char16
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_wstring_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_wstring_builder(
            uint32_t bound) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a sequence
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.4 this method is
     *         called `create_sequence_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] type @ref DynamicType which becomes the element type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_sequence_builder(
            const DynamicType& type,
            uint32_t bound = MAX_ELEMENTS_COUNT) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an array
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.1 this method is
     *         called `create_array_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] type @ref DynamicType which becomes the element type
     * @param[in] bounds `uint32_t` representing the desired dimensions
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_array_builder(
            const DynamicType& type,
            const std::vector<uint32_t>& bounds) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a map
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.3 this method is
     *         called `create_map_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] key_type @ref DynamicType which becomes the map's key type
     * @param[in] value_type @ref DynamicType which becomes the map's value type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_map_builder(
            const DynamicType& key_type,
            const DynamicType& value_type,
            uint32_t bound = MAX_ELEMENTS_COUNT) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitmask
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.2 this method is
     *         called `create_bitmask_type` which is misguiding. It was renamed to simplify interface usage.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_bitmask_builder(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_bitset_builder(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an alias
     * @param[in] base_type @ref DynamicType to be referenced
     * @param[in] sName new alias name
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_alias_builder(
            const DynamicType& base_type,
            const std::string& sName);

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an enum
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_enum_builder();

    /**
     * Returns a @ref DynamicTypeBuilder associated with a `TypeKind::TK_STRUCTURE`
     * @return @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_struct_builder() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a subclass
     * @param[in] parent_type @ref DynamicType identifying the desired superclass
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_child_struct_builder(
            const DynamicType& parent_type);

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a union
     * @param[in] discriminator_type @ref DynamicType associated to the union's discriminator
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder_ptr create_union_builder(
            const DynamicType& discriminator_type);

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an annotation
     * @param[in] name string annotation identifier
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicType_ptr create_annotation_primitive(
            const std::string& name);

    //! returns type instantiation of the @ref DynamicTypeBuilderFactory::create_alias_type builder
    RTPS_DllAPI DynamicType_ptr create_alias_type(
            const DynamicType& base_type,
            const std::string& sName);

    //! returns the cache type associated to create_int16_builder()
    RTPS_DllAPI DynamicType_ptr create_int16_type();

    //! returns the cache type associated to create_uint16_builder()
    RTPS_DllAPI DynamicType_ptr create_uint16_type();

    //! returns the cache type associated to create_int32_builder()
    RTPS_DllAPI DynamicType_ptr create_int32_type();

    //! returns the cache type associated to create_uint32_builder()
    RTPS_DllAPI DynamicType_ptr create_uint32_type();

    //! returns the cache type associated to create_int64_builder()
    RTPS_DllAPI DynamicType_ptr create_int64_type();

    //! returns the cache type associated to create_uint64_builder()
    RTPS_DllAPI DynamicType_ptr create_uint64_type();

    //! returns the cache type associated to create_float32_builder()
    RTPS_DllAPI DynamicType_ptr create_float32_type();

    //! returns the cache type associated to create_float64_builder()
    RTPS_DllAPI DynamicType_ptr create_float64_type();

    //! returns the cache type associated to create_float128_builder()
    RTPS_DllAPI DynamicType_ptr create_float128_type();

    //! returns the cache type associated to create_char8_builder()
    RTPS_DllAPI DynamicType_ptr create_char8_type();

    //! returns the cache type associated to create_char16_builder()
    RTPS_DllAPI DynamicType_ptr create_char16_type();

    //! returns the cache type associated to create_bool_builder()
    RTPS_DllAPI DynamicType_ptr create_bool_type();

    //! returns the cache type associated to create_byte_type()
    RTPS_DllAPI DynamicType_ptr create_byte_type();

    //! returns the cache type associated to create_string_builder()
    RTPS_DllAPI DynamicType_ptr create_string_type(
            uint32_t bound = MAX_STRING_LENGTH) noexcept;

    //! returns the cache type associated to create_wstring_builder()
    RTPS_DllAPI DynamicType_ptr create_wstring_type(
            uint32_t bound = MAX_STRING_LENGTH) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicType_ptr create_bitset_type(
            uint32_t bound);

    RTPS_DllAPI void build_type_identifier(
            const TypeDescriptor& descriptor,
            TypeIdentifier& identifier,
            bool complete = true) const;

    RTPS_DllAPI void build_type_object(
            const TypeDescriptor& descriptor,
            TypeObject& object,
            bool complete = true,
            bool force = false) const;

    RTPS_DllAPI bool is_empty() const;

    //! ostream ancillary
    static const int indentation_index; //!< indentation
    static const int object_index; //!< previous object in << stack
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_H
