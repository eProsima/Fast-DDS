/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef OMG_DDS_CORE_XTYPES_COLLECTION_TYPE_HPP_
#define OMG_DDS_CORE_XTYPES_COLLECTION_TYPE_HPP_

#include <dds/core/xtypes/DynamicType.hpp>

namespace dds {
namespace core {
namespace xtypes {

/// \brief DynamicType representing collection of elements.
/// This is the base abstract class for all collections.
/// A CollectionType represents a TypeKind::COLLECTION_TYPE.
class CollectionType : public DynamicType
{
public:
    /// \brief Get the content type of the collection.
    /// \returns a DynamicType represents the content type.
    const DynamicType& content_type() const { return *content_; }

    /// \brief Given an instance that represents a collection,
    /// it gets the instance at index location.
    /// \param[in] instance Where the collection is located.
    /// \param[in] index Index for the instance requested.
    /// \returns an instance representing a content_type() at index position.
    virtual uint8_t* get_instance_at(uint8_t* instance, size_t index) const = 0;

    /// \brief Get the size of an instance that represents a collection.
    /// \param[in] instance Where the collection is located.
    /// \returns The collection size
    virtual size_t get_instance_size(const uint8_t* instance) const = 0;

protected:
    CollectionType(
            TypeKind kind,
            const std::string& name,
            DynamicType::Ptr&& content)
        : DynamicType(kind, name)
        , content_(std::move(content))
    {}

private:
    DynamicType::Ptr content_;
};

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_COLLECTION_TYPE_HPP_
