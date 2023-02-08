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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTRACKER_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTRACKER_HPP

#include <mutex>
#include <set>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicTypeBuilderImpl;
class DynamicTypeImpl;
class DynamicDataImpl;

enum class type_tracking
{
    none = 0,
    partial,
    complete
};

namespace detail {

template<type_tracking mode>
struct dynamic_tracker_data;

template<>
struct dynamic_tracker_data<type_tracking::none> {};

template<>
struct dynamic_tracker_data<type_tracking::partial>
{
    std::set<const DynamicTypeBuilderImpl*> builders_list_; /*!< Collection of active DynamicTypeBuilderImpl instances */
    std::set<const DynamicTypeImpl*> types_list_; /*!< Collection of active DynamicTypeImpl instances */
    std::set<const DynamicDataImpl*> data_list_; /*!< Collection of active DynamicDataImpl instances */
    std::mutex mutex_; /*!< atomic access to the collections */
};

template<>
struct dynamic_tracker_data<type_tracking::complete>
    : public dynamic_tracker_data<type_tracking::partial>
{
    std::set<const DynamicTypeBuilderImpl*> primitive_builders_list_; /*!< Collection of static builder instances */
    std::set<const DynamicTypeImpl*> primitive_types_list_; /*!< Collection of static type instances */
};

} // namespace detail

/**
 * Interface use to track dynamic objects lifetime
 */
template<type_tracking mode>
class dynamic_tracker
    : protected detail::dynamic_tracker_data<mode>
{
    friend class DynamicTypeBuilderFactoryImpl;
    friend class DynamicTypeBuilderImpl;
    friend class DynamicDataFactoryImpl;

    //! clear collection contents
    void reset_types() noexcept;
    void reset_datas() noexcept;
    void reset() noexcept
    {
        reset_types();
        reset_datas();
    }

    //! check if there are leakages
    bool is_type_empty() noexcept;
    bool is_data_empty() noexcept;
    bool is_empty() noexcept
    {
        return is_type_empty() && is_data_empty();
    }

    //! add primitive builder
    void add_primitive(
            const DynamicTypeBuilderImpl*) noexcept;
    //! add primitive types
    void add_primitive(
            const DynamicTypeImpl*) noexcept;
    //! add new builder
    bool add(
            const DynamicTypeBuilderImpl*) noexcept;
    //! remove builder
    bool remove(
            const DynamicTypeBuilderImpl*) noexcept;
    //! add new type
    bool add(
            const DynamicTypeImpl*) noexcept;
    //! remove type
    bool remove(
            const DynamicTypeImpl*) noexcept;
    //! add data
    bool add(
            const DynamicDataImpl*) noexcept;
    //! remove data
    bool remove(
            const DynamicDataImpl*) noexcept;

    // singleton creation
    static dynamic_tracker<mode>& get_dynamic_tracker()
    {
        static dynamic_tracker<mode> dynamic_tracker;
        return dynamic_tracker;
    }

};

#if NDEBUG
constexpr type_tracking selected_mode = type_tracking::none;
#else
constexpr type_tracking selected_mode = type_tracking::complete;
#endif // NDEBUG

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_DYNAMICTRACKER_HPP
