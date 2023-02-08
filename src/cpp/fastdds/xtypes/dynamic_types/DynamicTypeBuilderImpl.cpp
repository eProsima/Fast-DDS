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
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include "DynamicTypeBuilderImpl.hpp"
#include "DynamicTypeImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "MemberDescriptorImpl.hpp"
#include "TypeState.hpp"
#include <fastdds/dds/log/Log.hpp>

#include <tuple>

namespace eprosima {
namespace fastdds {
namespace dds  {

DynamicTypeBuilderImpl::DynamicTypeBuilderImpl(
        use_the_create_method)
{
}

DynamicTypeBuilderImpl::DynamicTypeBuilderImpl(
        use_the_create_method,
        const DynamicTypeBuilderImpl& builder)
{
    *this = builder;
}

DynamicTypeBuilderImpl::DynamicTypeBuilderImpl(
        use_the_create_method,
        const TypeState& descriptor,
        bool is_static)
    : TypeState(descriptor)
    , current_member_id_(0)
{
    // the factory should only create consistent builders
    assert(is_consistent());

    // if static create and register the one and only primitive type
    if (is_static)
    {
        // create on heap
#if _MSC_VER >= 1921
        instance_ = std::make_shared<DynamicTypeImpl>(
            DynamicTypeImpl::use_the_create_method{},
            *this);
#else
        instance_.reset(new DynamicTypeImpl(
                    DynamicTypeImpl::use_the_create_method{},
                    *this));
#endif // _MSC_VER

        // notify the tracker
        dynamic_tracker<selected_mode>::get_dynamic_tracker().add_primitive(instance_.get());
    }
}

void DynamicTypeBuilderImpl::after_construction(
        DynamicTypeImpl* type)
{
    dynamic_tracker<selected_mode>::get_dynamic_tracker().add(type);
}

void DynamicTypeBuilderImpl::before_destruction(
        DynamicTypeImpl* type)
{
    dynamic_tracker<selected_mode>::get_dynamic_tracker().remove(type);
}

DynamicTypeBuilderImpl::member_iterator DynamicTypeBuilderImpl::add_empty_member(
        uint32_t index,
        const std::string& name)
{
    // inheritance corrections
    uint32_t offset = 0;

    if (base_type_)
    {
        offset = resolve_alias_type(*base_type_).get_member_count();
    }

    // insert the new member
    member_iterator it;
    if (index >= members_.size() + offset)
    {
        // adjust index
        index = static_cast<uint32_t>(members_.size()) + offset;
        // at the end
        it = members_.emplace(members_.end(), index, name); //TODO(richiware) Review idnex
    }
    else
    {
        // adjust index
        int advance = std::max(int(index - offset), 0);
        index = offset + advance;

        // move all the others
        it = members_.begin();
        std::advance(it, advance);
        it = members_.emplace(it, index, name);
        // rename the others
        auto nit = it;
        for (auto i = index; nit != members_.end(); ++nit, ++i)
        {
            nit->set_index(i);
            assert(nit->get_index() == i);
        }
    }

    if (get_kind() == TK_BITMASK)
    {
        // preconditions check on add_member() public API
        assert(index < get_bounds(0));
        it->annotation_set_position(static_cast<uint16_t>(index));
    }

    return it;
}

ReturnCode_t DynamicTypeBuilderImpl::add_member(
        MemberDescriptorImpl&& descriptor) noexcept
{
    try
    {
        if (!descriptor.is_consistent(get_kind()))
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, The input descriptor isn't consistent.");
        }

        if (get_kind() != TK_ANNOTATION && get_kind() != TK_BITMASK
                && get_kind() != TK_ENUM && get_kind() != TK_STRUCTURE
                && get_kind() != TK_UNION && get_kind() != TK_BITSET)
        {
            std::ostringstream os;
            os << "Error adding member, the current type " << (eprosima::fastrtps::rtps::octet)get_kind() <<
                " doesn't support members.";

            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_PRECONDITION_NOT_MET,
            // TODO(richiware)           os.str());
        }

        auto member_name = descriptor.get_name();

        // Bitsets allow multiple empty members.
        if ( get_kind() != TK_BITSET && descriptor.get_name().empty())
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, missing proper name.");
        }

        if (!member_name.empty() && exists_member_by_name(member_name))
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, there is other member with the same name.");
        }

        auto member_id = descriptor.get_id();
        if (member_id != MEMBER_ID_INVALID && exists_member_by_id(member_id))
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, there is other member with the same id.");
        }

        if (!check_union_configuration(descriptor))
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, invalid union parameters.");
        }

        if (get_kind() == TK_BITMASK &&
                descriptor.get_id() >= get_bounds(0))
        {
            // TODO(richiware) throw std::system_error(
            // TODO(richiware)           RETCODE_BAD_PARAMETER,
            // TODO(richiware)           "Error adding member, out of bounds.");
        }

        auto it = add_empty_member(descriptor.get_index(), member_name);

        DynamicTypeMemberImpl& newMember = *it;
        // Copy all elements but keep the index
        descriptor.index_ = newMember.index_;
        newMember = std::move(descriptor);

        if (member_id == MEMBER_ID_INVALID)
        {
            do
            {
                // assing a new one
                member_id = current_member_id_;
            } // check and advance
            while (exists_member_by_id(current_member_id_++));

            newMember.set_id(member_id);
        }

        // update the indexes collections
        if (!member_name.empty()) // Don't store empty bitset members.
        {
            member_by_id_.insert(std::make_pair(member_id, &newMember));
            member_by_name_.insert(std::make_pair(member_name, &newMember));
        }

        // invalidate type object
        instance_.reset();
    }
    catch (std::system_error& e)
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, e.what());
        return e.code().value();
    }

    return RETCODE_OK;
}

std::shared_ptr<const DynamicTypeImpl> DynamicTypeBuilderImpl::build() const
{
    // check if an instance is already available
    // and is still valid
    if ( !(instance_ && *this == *instance_) && is_consistent(true))
    {
        try
        {
            builder_allocator al;
            // otherwise, create a new one. Check total consistency
#if _MSC_VER >= 1921
            // MSVC v142 can allocate on a single block
            instance_ = std::allocate_shared<DynamicTypeImpl>(
                al,
                DynamicTypeImpl::use_the_create_method{},
                *this);
#else
            using traits = std::allocator_traits<builder_allocator>;
            auto new_instance = al.allocate(sizeof(DynamicTypeImpl));
            traits::construct(
                al,
                new_instance,
                DynamicTypeImpl::use_the_create_method{},
                *this);
            instance_.reset(new_instance);
#endif // if _MSC_VER >= 1921
        }
        catch (const std::bad_alloc& e)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error building type. It couldn't be allocated: " << e.what());
            return {};
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type. The current descriptor isn't consistent.");
        instance_.reset();
        return {};
    }

    instance_->add_ref();
    return instance_;
}

bool DynamicTypeBuilderImpl::check_union_configuration(
        const MemberDescriptorImpl& descriptor)
{
    if (get_kind() == TK_UNION)
    {
        bool default_union_value = descriptor.is_default_union_value();
        if (!default_union_value && descriptor.get_union_labels().size() == 0)
        {
            return false;
        }
        for (auto& m : members_ )
        {
            // Check that there isn't any member as default label and that there isn't any member with the same case.
            if ((default_union_value && m.is_default_union_value()) ||
                    !descriptor.check_union_labels(m.get_union_labels()))
            {
                return false;
            }
        }
    }
    return true;
}

void DynamicTypeBuilderImpl::clear()
{
    TypeState::clean();
    current_member_id_ = 0;
    instance_.reset();
}

bool DynamicTypeBuilderImpl::is_discriminator_type() const
{
    auto base = get_base_type();
    auto kind = get_kind();

    if (kind == TK_ALIAS && base)
    {
        return base->is_discriminator_type();
    }
    return kind == TK_BOOLEAN || kind == TK_BYTE || kind == TK_INT16 ||
           kind == TK_INT32 || kind == TK_INT64 || kind == TK_UINT16 ||
           kind == TK_UINT32 || kind == TK_UINT64 || kind == TK_FLOAT32 ||
           kind == TK_FLOAT64 || kind == TK_FLOAT128 || kind == TK_CHAR8 ||
           kind == TK_CHAR16 || kind == TK_STRING8 || kind == TK_STRING16 ||
           kind == TK_ENUM || kind == TK_BITMASK;
}

ReturnCode_t DynamicTypeBuilderImpl::delete_type(
        const DynamicTypeImpl& type) noexcept
{
    type.release();
    return RETCODE_OK;
}

const DynamicTypeImpl& DynamicTypeBuilderImpl::create_copy(
        const DynamicTypeImpl& type) noexcept
{
    // increase external reference counting
    type.add_ref();
    return type;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
