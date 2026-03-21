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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATAFACTORY_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATAFACTORY_HPP

#include <memory>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicDataFactory : public std::enable_shared_from_this<DynamicDataFactory>
{
public:

    using _ref_type = typename traits<DynamicDataFactory>::ref_type;

    /*!
     * Returns the singleton factory object
     * @remark This method is non thread-safe.
     * @return @ref DynamicDataFactory reference.
     */
    FASTDDS_EXPORTED_API static traits<DynamicDataFactory>::ref_type get_instance();

    /*!
     * Resets the singleton reference.
     * @return @ref ReturnCode_t
     * @retval RETCODE_BAD_PARAMETER if singleton reference is currently nil.
     * @retval RETCODE_OK otherwise.
     */
    FASTDDS_EXPORTED_API static ReturnCode_t delete_instance();

    /**
     * Creates a new @ref DynamicData reference based on the given @ref DynamicType reference.
     * All objects returned by this operation should eventually be deleted by calling delete_data.
     * @param [in] type @ref DynamicType reference associated.
     * @return new @ref DynamicData reference
     */
    FASTDDS_EXPORTED_API virtual traits<DynamicData>::ref_type create_data(
            traits<DynamicType>::ref_type type) = 0;

    /**
     * Resets the internal reference if it is cached.
     * @param [in] data @ref DynamicData reference whose internal cached reference to reset.
     * @return standard ReturnCode_t
     * @retval RETCODE_BAD_PARAMETER if reference is nil.
     * @retval RETCODE_OK is otherwise returned.
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t delete_data(
            traits<DynamicData>::ref_type& data) = 0;

protected:

    virtual ~DynamicDataFactory() = default;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICDATAFACTORY_HPP
