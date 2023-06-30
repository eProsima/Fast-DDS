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

#include <dynamic-types/v1_3/DynamicTypeBuilderFactoryImpl.hpp>
#include <dynamic-types/v1_3/DynamicTypeBuilderImpl.hpp>
#include <dynamic-types/v1_3/DynamicTypeImpl.hpp>
#include <dynamic-types/v1_3/DynamicTypeMemberImpl.hpp>
#include <dynamic-types/v1_3/MemberDescriptorImpl.hpp>
#include <dynamic-types/v1_3/TypeState.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <tuple>

using namespace eprosima::fastrtps::types::v1_3;
using eprosima::fastrtps::types::ReturnCode_t;

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
        instance_ = std::make_shared<DynamicTypeImpl>(
            DynamicTypeImpl::use_the_create_method{},
            *this);

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
        it = members_.emplace(members_.end(), index, name);
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

    if (get_kind() == TypeKind::TK_BITMASK)
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
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, The input descriptor isn't consistent.");
        }

        if (get_kind() != TypeKind::TK_ANNOTATION && get_kind() != TypeKind::TK_BITMASK
                && get_kind() != TypeKind::TK_ENUM && get_kind() != TypeKind::TK_STRUCTURE
                && get_kind() != TypeKind::TK_UNION && get_kind() != TypeKind::TK_BITSET)
        {
            std::ostringstream os;
            os << "Error adding member, the current type " << (octet)get_kind() << " doesn't support members.";

            throw std::system_error(
                      ReturnCode_t::RETCODE_PRECONDITION_NOT_MET,
                      os.str());
        }

        auto member_name = descriptor.get_name();

        // Bitsets allow multiple empty members.
        if ( get_kind() != TypeKind::TK_BITSET && descriptor.get_name().empty())
        {
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, missing proper name.");
        }

        if (!member_name.empty() && exists_member_by_name(member_name))
        {
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, there is other member with the same name.");
        }

        auto member_id = descriptor.get_id();
        if (member_id != MEMBER_ID_INVALID && exists_member_by_id(member_id))
        {
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, there is other member with the same id.");
        }

        if (!check_union_configuration(descriptor))
        {
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, invalid union parameters.");
        }

        if (get_kind() == TypeKind::TK_BITMASK &&
                descriptor.get_id() >= get_bounds(0))
        {
            throw std::system_error(
                      ReturnCode_t::RETCODE_BAD_PARAMETER,
                      "Error adding member, out of bounds.");
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

    return ReturnCode_t::RETCODE_OK;
}

const DynamicTypeImpl* DynamicTypeBuilderImpl::build() const
{
    // check if an instance is already available
    // and is still valid
    if ( !(instance_ && *this == *instance_) && is_consistent(true))
    {
        try
        {
            // otherwise, create a new one. Check total consistency
            instance_ = std::allocate_shared<DynamicTypeImpl>(
                builder_allocator{},
                DynamicTypeImpl::use_the_create_method{},
                *this);
        }
        catch(const std::bad_alloc& e)
        {
            EPROSIMA_LOG_ERROR(DYN_TYPES,
                    "Error building type. It couldn't be allocated: " << e.what());
            return nullptr;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(DYN_TYPES, "Error building type. The current descriptor isn't consistent.");
        instance_.reset();
        return nullptr;
    }

    instance_->add_ref();
    return instance_.get();
}

bool DynamicTypeBuilderImpl::check_union_configuration(
        const MemberDescriptorImpl& descriptor)
{
    if (get_kind() == TypeKind::TK_UNION)
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

    if (kind == TypeKind::TK_ALIAS && base)
    {
        return base->is_discriminator_type();
    }
    return kind == TypeKind::TK_BOOLEAN || kind == TypeKind::TK_BYTE || kind == TypeKind::TK_INT16 ||
           kind == TypeKind::TK_INT32 || kind == TypeKind::TK_INT64 || kind == TypeKind::TK_UINT16 ||
           kind == TypeKind::TK_UINT32 || kind == TypeKind::TK_UINT64 || kind == TypeKind::TK_FLOAT32 ||
           kind == TypeKind::TK_FLOAT64 || kind == TypeKind::TK_FLOAT128 || kind == TypeKind::TK_CHAR8 ||
           kind == TypeKind::TK_CHAR16 || kind == TypeKind::TK_STRING8 || kind == TypeKind::TK_STRING16 ||
           kind == TypeKind::TK_ENUM || kind == TypeKind::TK_BITMASK;
}

ReturnCode_t DynamicTypeBuilderImpl::delete_type(
        const DynamicTypeImpl* type) noexcept
{
    if (type != nullptr)
    {
        type->release();
        return ReturnCode_t::RETCODE_OK;
    }

    return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
}

const DynamicTypeImpl* DynamicTypeBuilderImpl::create_copy(
        const DynamicTypeImpl& type) noexcept
{
    // increase external reference counting
    type.add_ref();
    return &type;
}

void DynamicTypeBuilderImpl::external_dynamic_object_deleter(const DynamicTypeBuilderImpl* pDT)
{
    if (pDT)
    {
        pDT->release();
    }
}

void DynamicTypeBuilderImpl::internal_dynamic_object_deleter(const DynamicTypeBuilderImpl* pDT)
{
    if (pDT)
    {
        std::default_delete<const DynamicTypeBuilderImpl> del;
        del(pDT);
    }
}

void (*eprosima::fastrtps::types::v1_3::dynamic_object_deleter(
        const DynamicTypeBuilderImpl* pDT))(const DynamicTypeBuilderImpl*)
{
   if ( pDT != nullptr)
   {
        if (pDT->use_count())
        {
            return DynamicTypeBuilderImpl::external_dynamic_object_deleter;
        }
        else
        {
            return DynamicTypeBuilderImpl::internal_dynamic_object_deleter;
        }
   }

   return nullptr;
}
