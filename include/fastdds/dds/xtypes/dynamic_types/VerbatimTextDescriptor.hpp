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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__VERBATIMTEXTDESCRIPTOR_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__VERBATIMTEXTDESCRIPTOR_HPP

#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class FASTDDS_EXPORTED_API VerbatimTextDescriptor
{
public:

    using _ref_type = typename traits<VerbatimTextDescriptor>::ref_type;

    /*!
     * Returns the location within the generated output at which the output text should be inserted.
     * @return The location.
     */
    virtual std::string& placement() = 0;

    /*!
     * Returns the location within the generated output at which the output text should be inserted.
     * @return The location.
     */
    virtual const std::string& placement() const = 0;

    /*!
     * Sets the location within the generated output at which the output text should be inserted.
     * @param [in] placement The location.
     */
    virtual void placement(
            const std::string& placement) = 0;

    /*!
     * Sets the location within the generated output at which the output text should be inserted.
     * @param [in] placement The location.
     */
    virtual void placement(
            std::string&& placement) = 0;

    /*!
     * Returns the literal output text.
     * @return The text.
     */
    virtual std::string& text() = 0;

    /*!
     * Returns the literal output text.
     * @return The text.
     */
    virtual const std::string& text() const = 0;

    /*!
     * Sets the literal output text.
     * @param [in] text The text.
     */
    virtual void text(
            const std::string& text) = 0;

    /*!
     * Sets the literal output text.
     * @param [in] text The text.
     */
    virtual void text(
            std::string&& text) = 0;

    /*!
     * Overwrites the contents of this descriptor with those of another descriptor.
     * @param [in] descriptor reference.
     * @return @ref ReturnCode_t
     * @retval RETCODE_OK when the copy was successful.
     * @retval RETCODE_BAD_PARAMETER when descriptor reference is nil.
     */
    virtual ReturnCode_t copy_from(
            traits<VerbatimTextDescriptor>::ref_type descriptor) = 0;

    /*!
     * Compares.
     * @param [in] descriptor reference to compare to.
     * @return \b bool `true` on equality
     */
    virtual bool equals(
            traits<VerbatimTextDescriptor>::ref_type descriptor) = 0;

    /*!
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @return \b bool `true` if consistent.
     */
    virtual bool is_consistent() = 0;

protected:

    VerbatimTextDescriptor() = default;

    VerbatimTextDescriptor(
            const VerbatimTextDescriptor& type) = default;

    VerbatimTextDescriptor(
            VerbatimTextDescriptor&& type) = default;

    virtual ~VerbatimTextDescriptor() = default;

private:

    VerbatimTextDescriptor& operator =(
            const VerbatimTextDescriptor& type) = delete;

    VerbatimTextDescriptor& operator =(
            VerbatimTextDescriptor&& type) = delete;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__VERBATIMTEXTDESCRIPTOR_HPP
