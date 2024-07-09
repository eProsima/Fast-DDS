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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__ANNOTATIONDESCRIPTOR_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__ANNOTATIONDESCRIPTOR_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;

class FASTDDS_EXPORTED_API AnnotationDescriptor
{
public:

    using _ref_type = typename traits<AnnotationDescriptor>::ref_type;

    /*!
     * Returns a reference to the type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type type() const = 0;

    /*!
     * Returns a reference to the type. The reference can be nil.
     * @return @ref DynamicType reference.
     */
    virtual traits<DynamicType>::ref_type& type() = 0;

    /*!
     * Modifies the underlying type reference.
     * @param [in] type @ref DynamicType reference.
     */
    virtual void type(
            traits<DynamicType>::ref_type type) = 0;

    /**
     * Getter given a key for the @b value property.
     * @param [inout] value The value.
     * @param [in] key Key used to retrieve the value.
     * @return @ref ReturnCode_t returns operation success
     */
    virtual ReturnCode_t get_value(
            ObjectName& value,
            const ObjectName& key) = 0;

    /**
     * Getter for all the values.
     * @param [inout] value @ref Parameters interface to the strings map.
     * @return @ref ReturnCode_t returns operation success.
     */
    virtual ReturnCode_t get_all_value(
            Parameters& value) = 0;

    /**
     * Setter given a key for the @b value property.
     * @param [in] key null terminated string
     * @param [in] value null terminated string
     * @return @ref ReturnCode_t returns operation success
     */
    virtual ReturnCode_t set_value(
            const ObjectName& key,
            const ObjectName& value) = 0;

    /**
     * Overwrites the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.3.1)
     * @param [in] descriptor object
     * @return standard @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    virtual ReturnCode_t copy_from(
            traits<AnnotationDescriptor>::ref_type descriptor) = 0;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.3.2
     * @param [in] descriptor reference.
     * @return \b bool `true` on equality
     */
    virtual bool equals(
            traits<AnnotationDescriptor>::ref_type descriptor) = 0;

    /*!
     * Indicates whether the states of all of this descriptor's properties are consistent according with the [standard]
     * section \b 7.5.2.3.3.
     * @return \b bool `true` if consistent.
     */
    virtual bool is_consistent() = 0;

protected:

    AnnotationDescriptor() = default;

    AnnotationDescriptor(
            const AnnotationDescriptor&) = default;

    AnnotationDescriptor(
            AnnotationDescriptor&&) = default;

    virtual ~AnnotationDescriptor() = default;

private:

    AnnotationDescriptor& operator =(
            const AnnotationDescriptor&) = delete;

    AnnotationDescriptor& operator =(
            AnnotationDescriptor&&) = delete;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__ANNOTATIONDESCRIPTOR_HPP
