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

#include "DynamicDataFactoryImpl.hpp"
#include "DynamicTracker.hpp"
#include "DynamicTypeImpl.hpp"
#include "DynamicDataImpl.hpp"
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

void DynamicDataFactoryImpl::after_construction(
        DynamicDataImpl* d)
{
    dynamic_tracker<selected_mode>::get_dynamic_tracker().add(d);
}

void DynamicDataFactoryImpl::before_destruction(
        DynamicDataImpl* d)
{
    dynamic_tracker<selected_mode>::get_dynamic_tracker().remove(d);
}

void DynamicDataFactoryImpl::reset()
{
    dynamic_tracker<selected_mode>::get_dynamic_tracker().reset_datas();
}

bool DynamicDataFactoryImpl::is_empty() const
{
    return dynamic_tracker<selected_mode>::get_dynamic_tracker().is_data_empty();
}

DynamicDataFactoryImpl::~DynamicDataFactoryImpl()
{
    assert(is_empty());
    reset();
}

DynamicDataFactoryImpl& DynamicDataFactoryImpl::get_instance() noexcept
{
    // C++ standard requires preserve global construction order
    // make sure the dynamic tracker lifespan is larger than the factory one
    dynamic_tracker<selected_mode>::get_dynamic_tracker();

    // C++11 guarantees the construction to be atomic
    static DynamicDataFactoryImpl instance;
    return instance;
}

ReturnCode_t DynamicDataFactoryImpl::delete_instance() noexcept
{
    get_instance().reset();
    return RETCODE_OK;
}

template<class ... Args>
std::shared_ptr<DynamicDataImpl> DynamicDataFactoryImpl::create_data_impl(
        Args... args) noexcept
{
    try
    {
        std::shared_ptr<DynamicDataImpl> sp;
        auto& al = DynamicDataFactoryImpl::get_instance().get_allocator();

#if _MSC_VER >= 1921
        // MSVC v142 can allocate on a single block
        sp = std::allocate_shared<DynamicDataImpl>(
            al,
            DynamicDataImpl::use_the_create_method{},
            std::forward<Args>(args)...);
#else
        using traits = std::allocator_traits<builder_allocator>;
        auto new_instance = al.allocate(sizeof(DynamicDataImpl));
        traits::construct(
            al,
            new_instance,
            DynamicDataImpl::use_the_create_method{},
            std::forward<Args>(args)...);

        sp.reset(new_instance);
#endif // if _MSC_VER >= 1921
        // Keep alive on external references
        sp->add_ref();
        return sp;
    }
    catch (const std::bad_alloc& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building data object. Allocation failed: " << e.what());
    }

    return {};
}

std::shared_ptr<DynamicDataImpl> DynamicDataFactoryImpl::create_data(
        const DynamicTypeImpl& type) noexcept
{
    return create_data_impl(type);
}

std::shared_ptr<DynamicDataImpl> DynamicDataFactoryImpl::create_copy(
        const DynamicDataImpl& data) noexcept
{
    return create_data_impl(data);
}

std::shared_ptr<DynamicDataImpl> DynamicDataFactoryImpl::create_copy(
        DynamicDataImpl&& data) noexcept
{
    return create_data_impl(std::move(data));
}

ReturnCode_t DynamicDataFactoryImpl::delete_data(
        const DynamicDataImpl& data) noexcept
{
    return DynamicDataImpl::delete_data(data);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
