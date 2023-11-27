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

#include "AnnotationManager.hpp"
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

AnnotationDescriptor::AnnotationDescriptor(
        const AnnotationDescriptor& type) noexcept
    : type_(type.get_type())
    , map_(type.map_)
{
}

AnnotationDescriptor::AnnotationDescriptor(
        AnnotationDescriptor&& type) noexcept
    : type_(type.type_)
    , map_(std::move(map_))
{
}

AnnotationDescriptor::~AnnotationDescriptor() noexcept
{
    reset_type();
}

AnnotationDescriptor& AnnotationDescriptor::operator =(
        const AnnotationDescriptor& type) noexcept
{
    type_ = type.get_type();
    map_ = type.map_;
    return *this;
}

AnnotationDescriptor& AnnotationDescriptor::operator =(
        AnnotationDescriptor&& type) noexcept
{
    type_ = type.type_;
    map_ = std::move(type.map_);
    return *this;
}

bool AnnotationDescriptor::operator ==(
        const AnnotationDescriptor& other) const noexcept
{
    return ( type_ == other.type_
           || (type_ != nullptr && other.type_ != nullptr && *type_ == *other.type_))
           && map_ == other.map_;
}

const DynamicType* AnnotationDescriptor::get_type() const noexcept
{
    return nullptr == type_ ?
           nullptr :
           DynamicTypeBuilderFactory::get_instance().create_copy(*type_);
}

void AnnotationDescriptor::set_type(
        const DynamicType& type) noexcept
{
    reset_type();
    type_ = DynamicTypeBuilderFactory::get_instance().create_copy(type);
}

void AnnotationDescriptor::set_type(
        const DynamicType* type) noexcept
{
    reset_type();
    type_ = type;
}

void AnnotationDescriptor::reset_type() noexcept
{
    if (type_ != nullptr)
    {
        DynamicTypeBuilderFactory::get_instance().delete_type(type_);
    }

    type_ = nullptr;
}

ReturnCode_t AnnotationDescriptor::get_value(
        ObjectName& value,
        const ObjectName& key) const noexcept
{
    ReturnCode_t ret_code = RETCODE_BAD_PARAMETER;
    auto value_it =  map_.find(key);

    if (map_.end() != value_it)
    {
        value = value_it->second;
        ret_code = RETCODE_OK;
    }

    return ret_code;
}

ReturnCode_t AnnotationDescriptor::set_value(
        const ObjectName& key,
        const ObjectName& value) noexcept
{
    map_[key] = value;
    return RETCODE_OK;
}

ReturnCode_t AnnotationDescriptor::get_all_value(
        Parameters& value) const noexcept
{
    value = map_;
    return RETCODE_OK;
}

ReturnCode_t AnnotationDescriptor::copy_from(
        const AnnotationDescriptor& other) noexcept
{
    this->operator =(other);
    return RETCODE_OK;
}

bool AnnotationDescriptor::equals(
        const AnnotationDescriptor& other) const noexcept
{
    return this->operator ==(other);
}

bool AnnotationDescriptor::is_consistent() const noexcept
{
    if (type_ != nullptr && type_->get_kind() != TK_ANNOTATION)
    {
        return false;
    }

    //TODO: Check consistency of value_
    return true;
}

ReturnCode_t Annotations::get(
        AnnotationDescriptor& d,
        uint32_t pos) const noexcept
{
    return AnnotationManager::get_manager(*this).get_annotation(d, pos);
}

//! get collection size
uint64_t Annotations::size() const noexcept
{
    const auto& manager = AnnotationManager::get_manager(*this);
    return manager.get_annotation_count();
}

//! check contents
bool Annotations::empty() const noexcept
{
    return !size();
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
