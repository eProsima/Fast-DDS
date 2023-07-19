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

#ifndef TYPES_1_3_DYNAMIC_DATA_FACTORY_IMPL_HPP
#define TYPES_1_3_DYNAMIC_DATA_FACTORY_IMPL_HPP

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/utils/custom_allocators.hpp>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicTypeImpl;
class DynamicDataImpl;

/**
 * This class is conceived as a singleton charged of creation of @ref DynamicDataImpl objects.
 * For simplicity direct primitive types instantiation is also possible.
 */
class DynamicDataFactoryImpl final
{
    using builder_allocator = eprosima::detail::BuilderAllocator<DynamicDataImpl, DynamicDataFactoryImpl, true>;

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
            DynamicDataImpl* b);

    //! allocator callback
    void before_destruction(
            DynamicDataImpl* b);

    // free any allocated resources
    void reset();

    // check if there are outstanding objects associated
    bool is_empty() const;

    DynamicDataFactoryImpl() = default;

    template<class ...Args>
    std::shared_ptr<DynamicDataImpl> create_data_impl(
            Args... args) noexcept;

public:

    ~DynamicDataFactoryImpl();

    /**
     * Returns the singleton factory object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @return @ref DynamicDataFactoryImpl &
     */
    static DynamicDataFactoryImpl& get_instance() noexcept;

    /**
     * Resets the state of the factory
     * @remark This method is thread-safe.
     * @return standard @ref ReturnCode_t
     */
    static ReturnCode_t delete_instance() noexcept;

    /**
     * Create a new @ref DynamicDataImpl object based on the given @ref TypeState state.
     * @remark This method is thread-safe.
     * @param[in] type @ref DynamicType associated
     * @return new @ref DynamicDataImpl object
     */
    std::shared_ptr<DynamicDataImpl> create_data(
            const DynamicTypeImpl& type) noexcept;

    /**
     * Create a new @ref DynamicDataImpl object based on the given @ref DynamicDataImpl object.
     * @remark This method is thread-safe.
     * @param[in] type @ref DynamicDataImpl object
     * @return new @ref DynamicDataImpl object
     */
    std::shared_ptr<DynamicDataImpl> create_copy(
            const DynamicDataImpl& data) noexcept;
    std::shared_ptr<DynamicDataImpl> create_copy(
            DynamicDataImpl&& data) noexcept;

    /**
     * Frees any framework resources associated with the given data according with [standard] section 7.5.2.10.2.
     * @remark This method is thread-safe.
     * @remark RAII will prevent memory leaks even if this method is not called.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicTypeImpl object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    ReturnCode_t delete_data(
            const DynamicDataImpl& data) noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_DATA_FACTORY_IMPL_HPP
