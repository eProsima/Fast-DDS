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

#ifndef _FASTDDS_XTYPES_DYNAMIC_TYPES_VERBATIM_TEXT_DESCRIPTOR_IMPL_HPP_
#define _FASTDDS_XTYPES_DYNAMIC_TYPES_VERBATIM_TEXT_DESCRIPTOR_IMPL_HPP_

#include <fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class VerbatimTextDescriptorImpl : public virtual VerbatimTextDescriptor
{
    std::string placement_;

    std::string text_;

public:

    VerbatimTextDescriptorImpl() noexcept = default;

    VerbatimTextDescriptorImpl(
            const VerbatimTextDescriptorImpl& descriptor) noexcept = default;

    VerbatimTextDescriptorImpl(
            VerbatimTextDescriptorImpl&& descriptor) noexcept = default;

    virtual ~VerbatimTextDescriptorImpl() noexcept = default;

    std::string& placement() noexcept override
    {
        return placement_;
    }

    const std::string& placement() const noexcept override
    {
        return placement_;
    }

    void placement(
            const std::string& placement) noexcept override
    {
        placement_ = placement;
    }

    virtual void placement(
            std::string&& placement) noexcept override
    {
        placement_ = std::move(placement);
    }

    std::string& text() noexcept override
    {
        return text_;
    }

    const std::string& text() const noexcept override
    {
        return text_;
    }

    void text(
            const std::string& text) noexcept override
    {
        text_ = text;
    }

    virtual void text(
            std::string&& text) noexcept override
    {
        text_ = std::move(text);
    }

    ReturnCode_t copy_from(
            traits<VerbatimTextDescriptor>::ref_type descriptor) noexcept override;

    ReturnCode_t copy_from(
            const VerbatimTextDescriptorImpl& descriptor) noexcept;

    bool equals(
            traits<VerbatimTextDescriptor>::ref_type descriptor) noexcept override;

    bool equals(
            VerbatimTextDescriptorImpl& descriptor) noexcept;

    bool is_consistent() noexcept override;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_XTYPES_DYNAMIC_TYPES_VERBATIM_TEXT_DESCRIPTOR_IMPL_HPP_
