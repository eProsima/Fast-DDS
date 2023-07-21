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

#ifndef TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP
#define TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP

#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;
class DynamicData;

class DynamicDataFactory final
{
    DynamicDataFactory() = default;

public:

    RTPS_DllAPI ~DynamicDataFactory() = default;

    /**
     * Returns the singleton factory object
     * @remark This method is thread-safe.
     * @remark The singleton is allocated using C++11 builtin double-checked locking lazy initialization.
     * @return @ref DynamicDataFactory&
     */
    RTPS_DllAPI static DynamicDataFactory& get_instance() noexcept;

    /**
     * Resets the state of the factory
     * @remark This method is thread-safe.
     * @return standard @ref ReturnCode_t
     */
    RTPS_DllAPI static ReturnCode_t delete_instance() noexcept;

    /**
     * Create a new @ref DynamicData object based on the given @ref DynamicType state.
     * @remark This method is thread-safe.
     * @param[in] type @ref DynamicType associated
     * @return new @ref DynamicData object
     */
    RTPS_DllAPI DynamicData* create_data(
            const DynamicType& type) noexcept;

    /**
     * Create a new @ref DynamicDataImpl object based on the given object.
     * @remark This method is thread-safe.
     * @param[in] type @ref DynamicDataImpl object
     * @return new @ref DynamicDataImpl object
     */
    RTPS_DllAPI DynamicData* create_copy(
            const DynamicData& data) noexcept;

    /**
     * Frees any framework resources associated with the given data according with [standard] section 7.5.2.10.2.
     * @remark This method is thread-safe.
     * @remark Non-primitive types will not be tracked by the framework after this call.
     * @param[in] type @ref DynamicData object whose resources to free
     * @return standard ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    RTPS_DllAPI ReturnCode_t delete_data(
            const DynamicData* pData) noexcept;

    // check if there are outstanding objects associated
    RTPS_DllAPI bool is_empty() const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_DATA_FACTORY_HPP
