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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONDESCRIPTORIMPL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONDESCRIPTORIMPL_HPP

#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class AnnotationDescriptorImpl : public virtual AnnotationDescriptor
{
    //! Reference to the annotation type.
    traits<DynamicType>::ref_type type_;

    //! Collection of keys and values.
    Parameters value_;

public:

    AnnotationDescriptorImpl()  = default;

    AnnotationDescriptorImpl(
            const AnnotationDescriptorImpl&)  = default;

    AnnotationDescriptorImpl(
            AnnotationDescriptorImpl&&)  = default;

    virtual ~AnnotationDescriptorImpl()  = default;

    traits<DynamicType>::ref_type type() const noexcept override
    {
        return type_;
    }

    traits<DynamicType>::ref_type& type() noexcept override
    {
        return type_;
    }

    void type(
            traits<DynamicType>::ref_type type) noexcept override
    {
        type_ = type;
    }

    ReturnCode_t get_value(
            ObjectName& value,
            const ObjectName& key) noexcept override;

    ReturnCode_t get_value(
            ObjectName& value,
            const ObjectName& key) const noexcept;

    ReturnCode_t get_all_value(
            Parameters& value) noexcept override;

    ReturnCode_t get_all_value(
            Parameters& value) const noexcept;

    ReturnCode_t set_value(
            const ObjectName& key,
            const ObjectName& value) noexcept override;

    ReturnCode_t copy_from(
            traits<AnnotationDescriptor>::ref_type descriptor) noexcept override;

    ReturnCode_t copy_from(
            const AnnotationDescriptorImpl& descriptor) noexcept;

    bool equals(
            traits<AnnotationDescriptor>::ref_type descriptor) noexcept override;

    bool equals(
            AnnotationDescriptorImpl& descriptor) noexcept;

    bool is_consistent() noexcept override;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_ANNOTATIONDESCRIPTORIMPL_HPP
