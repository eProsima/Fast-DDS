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

#ifndef TYPES_1_3_ANNOTATION_DESCRIPTOR_H
#define TYPES_1_3_ANNOTATION_DESCRIPTOR_H

#include <cstdint>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;
class AnnotationDescriptor;
class AnnotationDescriptorImpl;
class AnnotationManager;

class RTPS_DllAPI Parameters final
{
    using mapping = std::map<std::string, std::string>;
    mapping* map_ = nullptr;
    bool ownership_ = false;

    Parameters(mapping& map) : map_(&map) {}

    friend class AnnotationDescriptor;

public:

    Parameters();

    ~Parameters();

    Parameters(const Parameters& type) noexcept;

    Parameters(Parameters&& type) noexcept;

    Parameters& operator=(const Parameters& type) noexcept;

    Parameters& operator=(Parameters&& type) noexcept;

    bool operator ==(
            const Parameters& descriptor) const noexcept;

    bool operator !=(
            const Parameters& descriptor) const noexcept;

    /*
     * Retrieve values from keys:
     * @param key name
     * @return associated member or nullptr if not present
     */
    const char* operator[](const char* key) const noexcept;
    const char* at(const char* key) const noexcept;

    //! add a new element
    ReturnCode_t set_value(const char* key, const char* value) noexcept;

    //! get collection size
    uint64_t size() const noexcept;

    //! check contents
    bool empty() const noexcept;

    /*
     * Iterate over the key elements.
     * @param key name of the previous key. Use nullptr (default) to get the first key
     * @return next key name or nullptr as end marker
     */
    const char* next_key(const char* key = nullptr) const noexcept;

};

class RTPS_DllAPI AnnotationDescriptor final
{
    const DynamicType* type_ = nullptr;         //!< Member's Type.
    Parameters map_;                            //!< Mapping

    AnnotationDescriptor(Parameters::mapping& map) : map_(map) {}

    friend class AnnotationDescriptorImpl;

public:

    AnnotationDescriptor() noexcept = default;

    AnnotationDescriptor(const AnnotationDescriptor& type) noexcept;

    AnnotationDescriptor(AnnotationDescriptor&& type) noexcept;

    ~AnnotationDescriptor() noexcept;

    AnnotationDescriptor& operator=(const AnnotationDescriptor& type) noexcept;

    AnnotationDescriptor& operator=(AnnotationDescriptor&& type) noexcept;

    bool operator ==(
            const AnnotationDescriptor& other) const noexcept;

    bool operator !=(
            const AnnotationDescriptor& other) const noexcept;

    /**
     * Getter for @b type property (see [standard] table 49)
     * @return @ref DynamicType
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is ownership transference. The returned value must be released.
     */
    const DynamicType* get_type() const noexcept;

    /**
     * Modifies the underlying type by copy
     * @param[in] type reference
     * @attention There is no ownership transference.
     */
    void set_type(
                const DynamicType& type) noexcept;

    //! Clears the base type reference
    void reset_type() noexcept;

    /**
     * Getter given a key for the @b value property (see [standard] table 49)
     * @param[in] key null terminated string
     * @param[inout] error @ref ReturnCode_t returns operation success
     * @return value as a null terminated string.
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle linked to the annotation.
     */
    const char* get_value(const char* key, ReturnCode_t* error = nullptr) const noexcept;

    /**
     * Setter given a key for the @b value property (see [standard] table 49)
     * @param[in] key null terminated string
     * @param[in] value null terminated string
     * @return @ref ReturnCode_t returns operation success
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t set_value(const char* key, const char* value) noexcept;

    /**
     * Getter for the @b value property (see [standard] table 49)
     * @param[inout] error @ref ReturnCode_t returns operation success
     * @return @ref Parameters interface to the strings map
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     * @attention There is no ownership transference. Livecycle linked to the annotation.
     */
    const Parameters* get_all_value(ReturnCode_t* error = nullptr) const noexcept;

    /**
     * Overwrite the contents of this descriptor with those of another descriptor (see [standard] 7.5.2.3.1)
     * @param[in] descriptor object
     * @return standard @ref ReturnCode_t
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "OMG standard"
     */
    ReturnCode_t copy_from(const AnnotationDescriptor& other) noexcept;

    /**
     * State comparison according with the [standard] sections \b 7.5.2.3.2
     * @remarks using `==` and `!=` operators is more convenient
     * @param[in] descriptor object state to compare to
     * @return \b bool `true` on equality
     * [standard]: https://www.omg.org/spec/DDS-XTypes/1.3/ "to the OMG standard"
     */
    bool equals(const AnnotationDescriptor& other) const noexcept;

    /**
     * Indicates whether the states of all of this descriptor's properties are consistent.
     * @param type bool `true` if we search a consistent type
     * @remark consistency for a type is more restrictive than for a builder which may require
     *         members and annotations.
     * @return \b bool `true` if consistent
     */
    bool is_consistent() const noexcept;
};

class Annotations final
{
    // TODO: wait until the AnnotationManger is refactored

    friend class AnnotationManager;

    public:

    /*
     * Retrieve values:
     * @param pos uint64_t zero based position
     * @return associated member or nullptr if not present
     */
    RTPS_DllAPI AnnotationDescriptor* operator[](uint64_t pos) const noexcept;
    RTPS_DllAPI AnnotationDescriptor* at(uint64_t pos) const noexcept;

    //! get collection size
    RTPS_DllAPI uint64_t size() const noexcept;

    //! check contents
    RTPS_DllAPI bool empty() const noexcept;
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_ANNOTATION_DESCRIPTOR_H
