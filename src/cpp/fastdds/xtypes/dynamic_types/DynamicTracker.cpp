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

#include "DynamicTracker.hpp"
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastdds {
namespace dds {

// explicit instatiation of tracker methods for type_tracking::complete

template<>
void dynamic_tracker<type_tracking::complete>::reset_types() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    builders_list_.clear();
    types_list_.clear();
    primitive_builders_list_.clear();
    primitive_types_list_.clear();
}

template<>
void dynamic_tracker<type_tracking::complete>::reset_datas() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    data_list_.clear();
}

template<>
bool dynamic_tracker<type_tracking::complete>::is_type_empty() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    return builders_list_.empty() && types_list_.empty();
}

template<>
bool dynamic_tracker<type_tracking::complete>::is_data_empty() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    return data_list_.empty();
}

template<>
void dynamic_tracker<type_tracking::complete>::add_primitive(
        const DynamicTypeBuilderImpl* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    primitive_builders_list_.insert(pBuilder);
}

template<>
void dynamic_tracker<type_tracking::complete>::add_primitive(
        const DynamicTypeImpl* pType) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    primitive_types_list_.insert(pType);
}

template<>
bool dynamic_tracker<type_tracking::complete>::add(
        const DynamicTypeBuilderImpl* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!builders_list_.insert(pBuilder).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been inserted previously.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::complete>::remove(
        const DynamicTypeBuilderImpl* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!builders_list_.erase(pBuilder)
            && primitive_builders_list_.find(pBuilder) != primitive_builders_list_.end())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been already removed.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::complete>::add(
        const DynamicTypeImpl* pType) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!types_list_.insert(pType).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been inserted previously.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::complete>::remove(
        const DynamicTypeImpl* type) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!types_list_.erase(type)
            && primitive_types_list_.find(type) != primitive_types_list_.end())
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been already removed.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::complete>::add(
        const DynamicDataImpl* pData) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!data_list_.insert(pData).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given data has been inserted previously.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::complete>::remove(
        const DynamicDataImpl* data) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!data_list_.erase(data))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given data has been already removed.");
        return false;
    }
    return true;
}

#ifndef NDEBUG
// force instantiation for error checking
template class dynamic_tracker<type_tracking::complete>;
#endif // ifndef NDEBUG

// explicit instatiation of tracker methods for type_tracking::partial

template<>
void dynamic_tracker<type_tracking::partial>::reset_types() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    builders_list_.clear();
    types_list_.clear();
}

template<>
void dynamic_tracker<type_tracking::partial>::reset_datas() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    data_list_.clear();
}

template<>
bool dynamic_tracker<type_tracking::partial>::is_type_empty() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    return builders_list_.empty() && types_list_.empty();
}

template<>
bool dynamic_tracker<type_tracking::partial>::is_data_empty() noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    return data_list_.empty();
}

template<>
void dynamic_tracker<type_tracking::partial>::add_primitive(
        const DynamicTypeBuilderImpl*) noexcept
{
}

template<>
void dynamic_tracker<type_tracking::partial>::add_primitive(
        const DynamicTypeImpl*) noexcept
{
}

template<>
bool dynamic_tracker<type_tracking::partial>::add(
        const DynamicTypeBuilderImpl* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!builders_list_.insert(pBuilder).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been inserted previously.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::partial>::remove(
        const DynamicTypeBuilderImpl* pBuilder) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!builders_list_.erase(pBuilder))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type builder has been already removed.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::partial>::add(
        const DynamicTypeImpl* pType) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!types_list_.insert(pType).second)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been inserted previously.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::partial>::remove(
        const DynamicTypeImpl* type) noexcept
{
    std::lock_guard<std::mutex> _(mutex_);
    if (!types_list_.erase(type))
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "The given type has been already removed.");
        return false;
    }
    return true;
}

template<>
bool dynamic_tracker<type_tracking::partial>::add(
        const DynamicDataImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::partial>::remove(
        const DynamicDataImpl*) noexcept
{
    return true;
}

#ifndef NDEBUG
// force instantiation for error checking
template class dynamic_tracker<type_tracking::partial>;
#endif // ifndef NDEBUG

// explicit instatiation of tracker methods for type_tracking::none

template<>
void dynamic_tracker<type_tracking::none>::reset_types() noexcept
{
}

template<>
void dynamic_tracker<type_tracking::none>::reset_datas() noexcept
{
}

template<>
bool dynamic_tracker<type_tracking::none>::is_data_empty() noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::is_type_empty() noexcept
{
    return true;
}

template<>
void dynamic_tracker<type_tracking::none>::add_primitive(
        const DynamicTypeBuilderImpl*) noexcept
{
}

template<>
void dynamic_tracker<type_tracking::none>::add_primitive(
        const DynamicTypeImpl*) noexcept
{
}

template<>
bool dynamic_tracker<type_tracking::none>::add(
        const DynamicTypeBuilderImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::remove(
        const DynamicTypeBuilderImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::add(
        const DynamicTypeImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::remove(
        const DynamicTypeImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::add(
        const DynamicDataImpl*) noexcept
{
    return true;
}

template<>
bool dynamic_tracker<type_tracking::none>::remove(
        const DynamicDataImpl*) noexcept
{
    return true;
}

#ifndef NDEBUG
// force instantiation for error checking
template class dynamic_tracker<type_tracking::none>;
#endif // ifndef NDEBUG


} // namespace dds
} // namespace fastdds
} // namespace eprosima
