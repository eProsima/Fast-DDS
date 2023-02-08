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

Parameters::Parameters()
    : map_(new mapping)
{
}

Parameters::~Parameters()
{
    delete map_;
}

Parameters::Parameters(
        const Parameters& type) noexcept
    : map_(new mapping(*type.map_))
{
}

Parameters::Parameters(
        Parameters&& type) noexcept
    : map_(type.map_)
{
}

Parameters& Parameters::operator =(
        const Parameters& type) noexcept
{
    map_ = new mapping(*type.map_);
    return *this;
}

Parameters& Parameters::operator =(
        Parameters&& type) noexcept
{
    map_ = type.map_;
    return *this;
}

bool Parameters::operator ==(
        const Parameters& other) const noexcept
{
    return map_ == other.map_
           || (map_ != nullptr && other.map_ != nullptr && *map_ == *other.map_);
}

bool Parameters::operator !=(
        const Parameters& other) const noexcept
{
    return !this->operator ==(other);
}

const char* Parameters::operator [](
        const char* key) const noexcept
{
    if (nullptr != map_)
    {
        auto it = map_->find(key);
        if ( it != map_->cend())
        {
            return it->second.c_str();
        }
    }

    return nullptr;
}

const char* Parameters::at(
        const char* key) const noexcept
{
    return this->operator [](key);
}

ReturnCode_t Parameters::set_value(
        const char* key,
        const char* value) noexcept
{
    if (nullptr == key || nullptr == value )
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }

    if (nullptr == map_)
    {
        map_ = new(std::nothrow) mapping;

        if (nullptr == map_)
        {
            return RETCODE_OUT_OF_RESOURCES;
        }
    }

    (*map_)[key] = value;

    return RETCODE_OK;
}

uint64_t Parameters::size() const noexcept
{
    return map_ != nullptr ? map_->size() : 0u;
}

bool Parameters::empty() const noexcept
{
    return map_ != nullptr ? map_->empty() : true;
}

const char* Parameters::next_key(
        const char* key /*= nullptr*/) const noexcept
{
    if (nullptr == map_)
    {
        return nullptr;
    }

    mapping::const_iterator it;

    if (nullptr == key)
    {
        it = map_->cbegin();
    }
    else
    {
        it = map_->find(key);
    }

    if (it != map_->cend())
    {
        return it->second.c_str();
    }

    return nullptr;
}

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

const char* AnnotationDescriptor::get_value(
        const char* key,
        ReturnCode_t* error /*= nullptr*/) const noexcept
{
    const char* res = map_[key];

    if (error != nullptr)
    {
        *error = res != nullptr ? RETCODE_OK : RETCODE_BAD_PARAMETER;
    }

    return res;
}

ReturnCode_t AnnotationDescriptor::set_value(
        const char* key,
        const char* value) noexcept
{
    return map_.set_value(key, value);
}

const Parameters* AnnotationDescriptor::get_all_value(
        ReturnCode_t* error /*= nullptr*/) const noexcept
{
    *error = RETCODE_OK;
    return &map_;
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
