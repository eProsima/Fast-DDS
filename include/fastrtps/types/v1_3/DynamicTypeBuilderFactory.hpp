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

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/v1_3/TypeDescriptor.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeIdentifier;
class TypeObject;

namespace v1_3 {

class DynamicTypeBuilderFactoryImpl;

class DynamicTypeBuilderFactory final
{
    DynamicTypeBuilderFactory() = default;

    friend class DynamicTypeBuilderFactoryImpl;

public:

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
     * Retrieve the cached @ref DynamicType object associated to a given primitive
     * @remark This method is thread-safe.
     * @param[in] kind type identifying the primitive type to retrieve
     * @return @ref DynamicType object
     */
    RTPS_DllAPI const DynamicType* get_primitive_type(
            TypeKind kind) noexcept;

    /**
     * Retrieve a @ref DynamicTypeBuilder object associated to a given primitive
     * @remark This method is thread-safe.
     * @param[in] kind type identifying the primitive type to retrieve
     * @return @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI const DynamicTypeBuilder* create_primitive_type(
            TypeKind kind) noexcept;

    /**
     * Create a new @ref DynamicTypeBuilder object based on the given @ref TypeDescriptor state.
     * @remark This method is thread-safe.
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.6 this method is
     *         called `create_type` which is misguiding. Note it returns a builder associated with the type.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] td object state to copy
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_type(
            const TypeDescriptor& td) noexcept;

    /**
     * Create a new @ref DynamicTypeBuilder object based on the given @ref DynamicType object.
     * @remark This method is thread-safe.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] type @ref DynamicType object
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_type_copy(
            const DynamicType& type) noexcept;

    /**
     * Create a new @ref DynamicTypeBuilder object based on the given @ref DynamicTypeBuilder object.
     * @remark This method is thread-safe.
     * @remark This method will always create a new builder object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] type @ref DynamicTypeBuilder object
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_type_copy(
            const DynamicTypeBuilder& type) noexcept;

    /**
     * Create a new @ref DynamicType object based on the given @ref DynamicType object.
     * @remark This method is thread-safe.
     * @remark This method will always create a new object. In order to access primitive static allocated
     *         ones and avoid heap overhead use the `get_xxxx_type()` methods.
     * @param[in] type @ref DynamicType object
     * @return new @ref DynamicType object copy
     */
    RTPS_DllAPI const DynamicType* create_copy(
            const DynamicType& type) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bounded string type.
     * @remark The element type of the typed returned is a char8
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_string_type` which is misguiding. Note it returns a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI const DynamicTypeBuilder* create_string_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bounded wstring type.
     * @remark The element type of the typed returned is a char16
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.5 this method is
     *         called `create_wstring_type` which is misguiding. Note it returns a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI const DynamicTypeBuilder* create_wstring_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a sequence
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.4 this method is
     *         called `create_sequence_type` which is misguiding. Note it returns a builder.
     * @param[in] type @ref DynamicType which becomes the element type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_sequence_type(
            const DynamicType& type,
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an array
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.1 this method is
     *         called `create_array_type` which is misguiding. Note the return value is a builder.
     * @param[in] type @ref DynamicType which becomes the element type
     * @param[in] bounds `uint32_t` representing the desired dimensions
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_array_type(
            const DynamicType& type,
            const uint32_t* bounds,
            uint32_t count) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a map
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.3 this method is
     *         called `create_map_type` which is misguiding. Note the return value is a builder.
     * @param[in] key_type @ref DynamicType which becomes the map's key type
     * @param[in] value_type @ref DynamicType which becomes the map's value type
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_map_type(
            const DynamicType& key_type,
            const DynamicType& value_type,
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitmask
     * @remark In the [standard](https://www.omg.org/spec/DDS-XTypes/1.3/) section \b 7.5.2.2.2 this method is
     *         called `create_bitmask_type` which is misguiding. Note the return value is a builder.
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_bitmask_type(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_bitset_type(
            uint32_t bound = 32) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilderobject representing an alias
     * @param[in] base_type @ref DynamicTypeto be referenced
     * @param[in] sName new alias name
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_alias_type(
            const DynamicType& base_type,
            const char* sName) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an enum
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_enum_type() noexcept;

    /**
     * Returns a @ref DynamicTypeBuilder associated with a `TypeKind::TK_STRUCTURE`
     * @return @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_struct_type() noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a subclass
     * @param[in] parent_type @ref DynamicType identifying the desired superclass
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_child_struct_type(
            const DynamicType& parent_type) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a union
     * @param[in] discriminator_type @ref DynamicType associated to the union's discriminator
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI DynamicTypeBuilder* create_union_type(
            const DynamicType& discriminator_type) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing an annotation
     * @param[in] name string annotation identifier
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI const DynamicType* create_annotation_primitive(
            const char* name) noexcept;

    //! returns type instantiation of the @ref DynamicTypeBuilderFactory::create_alias_type builder
    RTPS_DllAPI const DynamicType* get_alias_type(
            const DynamicType& base_type,
            const char* sName) noexcept;

    //! returns the cache type associated to create_string_type()
    RTPS_DllAPI const DynamicType* get_string_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    //! returns the cache type associated to create_wstring_type()
    RTPS_DllAPI const DynamicType* get_wstring_type(
            uint32_t bound = LENGTH_UNLIMITED) noexcept;

    /**
     * Creates a new @ref DynamicTypeBuilder object representing a bitset
     * @param[in] bound `uint32_t` representing the maximum number of elements that may be stored.
     * @return new @ref DynamicTypeBuilder object
     */
    RTPS_DllAPI const DynamicType* get_bitset_type(
            uint32_t bound) noexcept;

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
            const DynamicType* type) noexcept;

    /**
     * Frees any framework resources associated with the given type according with [standard] section 7.5.2.2.10.
     * @remark This method is thread-safe.
     * @remark RAII will prevent memory leaks even if this method is not called.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicTypeBuilder object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI ReturnCode_t delete_type(
            const DynamicTypeBuilder* type) noexcept;

    //! alias of `create_type(TypeKind::TK_INT32)`
    RTPS_DllAPI const DynamicTypeBuilder* create_int32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT32)`
    RTPS_DllAPI const DynamicTypeBuilder* create_uint32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_INT16)`
    RTPS_DllAPI const DynamicTypeBuilder* create_int16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT16)`
    RTPS_DllAPI const DynamicTypeBuilder* create_uint16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_INT64)`
    RTPS_DllAPI const DynamicTypeBuilder* create_int64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_UINT64)`
    RTPS_DllAPI const DynamicTypeBuilder* create_uint64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT32)`
    RTPS_DllAPI const DynamicTypeBuilder* create_float32_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT64)`
    RTPS_DllAPI const DynamicTypeBuilder* create_float64_type() noexcept;

    //! alias of `create_type(TypeKind::TK_FLOAT128)`
    RTPS_DllAPI const DynamicTypeBuilder* create_float128_type() noexcept;

    //! alias of `create_type(TypeKind::TK_CHAR8)`
    RTPS_DllAPI const DynamicTypeBuilder* create_char8_type() noexcept;

    //! alias of `create_type(TypeKind::TK_CHAR16)`
    RTPS_DllAPI const DynamicTypeBuilder* create_char16_type() noexcept;

    //! alias of `create_type(TypeKind::TK_BOOLEAN)`
    RTPS_DllAPI const DynamicTypeBuilder* create_bool_type() noexcept;

    //! alias of `create_type(TypeKind::TK_BYTE)`
    RTPS_DllAPI const DynamicTypeBuilder* create_byte_type() noexcept;

    //! returns the cache type associated to create_int16_type()
    RTPS_DllAPI const DynamicType* get_int16_type() noexcept;

    //! returns the cache type associated to create_uint16_type()
    RTPS_DllAPI const DynamicType* get_uint16_type() noexcept;

    //! returns the cache type associated to create_int32_type()
    RTPS_DllAPI const DynamicType* get_int32_type() noexcept;

    //! returns the cache type associated to create_uint32_type()
    RTPS_DllAPI const DynamicType* get_uint32_type() noexcept;

    //! returns the cache type associated to create_int64_type()
    RTPS_DllAPI const DynamicType* get_int64_type() noexcept;

    //! returns the cache type associated to create_uint64_type()
    RTPS_DllAPI const DynamicType* get_uint64_type() noexcept;

    //! returns the cache type associated to create_float32_type()
    RTPS_DllAPI const DynamicType* get_float32_type() noexcept;

    //! returns the cache type associated to create_float64_type()
    RTPS_DllAPI const DynamicType* get_float64_type() noexcept;

    //! returns the cache type associated to create_float128_type()
    RTPS_DllAPI const DynamicType* get_float128_type() noexcept;

    //! returns the cache type associated to create_char8_type()
    RTPS_DllAPI const DynamicType* get_char8_type() noexcept;

    //! returns the cache type associated to create_char16_type()
    RTPS_DllAPI const DynamicType* get_char16_type() noexcept;

    //! returns the cache type associated to create_bool_type()
    RTPS_DllAPI const DynamicType* get_bool_type() noexcept;

    //! returns the cache type associated to get_byte_type()
    RTPS_DllAPI const DynamicType* get_byte_type() noexcept;

    /**
     * Create a @ref TypeIdentifier from a @ref DynamicTypeBuilder
     * @param bld @ref DynamicTypeBuilder object to transform
     * @param identifier @ref TypeIdentifier to populate
     * @param complete bool specify if TypeIdentifier should be complete
     */
    RTPS_DllAPI void build_type_identifier(
            const DynamicTypeBuilder& bld,
            TypeIdentifier& identifier,
            bool complete = true) const noexcept;

    /**
     * Create a @ref TypeIdentifier from a @ref DynamicType
     * @param bld @ref DynamicType object to transform
     * @param identifier @ref TypeIdentifier to populate
     * @param complete bool specify if TypeIdentifier should be complete
     */
    RTPS_DllAPI void build_type_identifier(
            const DynamicType& tp,
            TypeIdentifier& identifier,
            bool complete = true) const noexcept;

    /**
     * Create a @ref TypeObject from a @ref DynamicTypeBuilder
     * @param bld @ref DynamicTypeBuilder object to transform
     * @param identifier @ref TypeObject to populate
     * @param complete bool specify if TypeIdentifier should be complete
     * @param force bool specify if the object creation is mandatory
     */
    RTPS_DllAPI void build_type_object(
            const DynamicTypeBuilder& bld,
            TypeObject& object,
            bool complete = true,
            bool force = false) const noexcept;

    /**
     * Create a @ref TypeObject from a @ref DynamicType
     * @param bld @ref DynamicType object to transform
     * @param identifier @ref TypeObject to populate
     * @param complete bool specify if TypeIdentifier should be complete
     * @param force bool specify if the object creation is mandatory
     */
    RTPS_DllAPI void build_type_object(
            const DynamicType& tp,
            TypeObject& object,
            bool complete = true,
            bool force = false) const noexcept;

    //! Check if there are outstanding objects to free
    RTPS_DllAPI bool is_empty() const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_TYPE_BUILDER_FACTORY_H
