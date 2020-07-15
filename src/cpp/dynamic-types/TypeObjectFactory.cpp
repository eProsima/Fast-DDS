// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeNamesGenerator.h>
#include <fastrtps/types/BuiltinAnnotationsTypeObject.h>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/utils/md5.h>
#include <fastdds/dds/log/Log.hpp>
#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeObjectFactoryReleaser
{
public:
    ~TypeObjectFactoryReleaser()
    {
        TypeObjectFactory::delete_instance();
    }
};

static TypeObjectFactoryReleaser s_releaser;
static TypeObjectFactory* g_instance = nullptr;
TypeObjectFactory* TypeObjectFactory::get_instance()
{
    if (g_instance == nullptr)
    {
        g_instance = new TypeObjectFactory();
        g_instance->create_builtin_annotations();
    }
    return g_instance;
}

ReturnCode_t TypeObjectFactory::delete_instance()
{
    if (g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_ERROR;
}

TypeObjectFactory::TypeObjectFactory()
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    // Generate basic TypeIdentifiers
    TypeIdentifier* auxIdent;
    // TK_BOOLEAN:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_BOOLEAN);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BOOLEAN, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_BYTE, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT8, auxIdent));
    // TK_BYTE:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_BYTE);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT8, auxIdent));
    // TK_INT16:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_INT16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT16, auxIdent));
    // TK_INT32:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_INT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT32, auxIdent));
    // TK_INT64:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_INT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_INT64, auxIdent));
    // TK_UINT16:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_UINT16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT16, auxIdent));
    // TK_UINT32:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_UINT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT32, auxIdent));
    // TK_UINT64:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_UINT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_UINT64, auxIdent));
    // TK_FLOAT32:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_FLOAT32);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT32, auxIdent));
    // TK_FLOAT64:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_FLOAT64);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT64, auxIdent));
    // TK_FLOAT128:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_FLOAT128);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_FLOAT128, auxIdent));
    // TK_CHAR8:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_CHAR8);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR8, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_CHAR16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16, auxIdent));
    // TK_CHAR16:
    auxIdent = new TypeIdentifier();
    identifiers_created_.push_back(auxIdent);
    auxIdent->_d(TK_CHAR16);
    identifiers_.insert(std::pair<std::string, TypeIdentifier*>(TKNAME_CHAR16T, auxIdent));
}

TypeObjectFactory::~TypeObjectFactory()
{
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexInformations);
        for (TypeInformation* inf : informations_created_)
        {
            delete inf;
        }
        informations_.clear();
        informations_created_.clear();
    }
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
        identifiers_.clear();
        complete_identifiers_.clear();

        for (TypeIdentifier* id : identifiers_created_)
        {
            delete id;
        }
        identifiers_created_.clear();
    }
    {
        std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
        auto obj_it = objects_.begin();
        while (obj_it != objects_.end())
        {
            delete (obj_it->second);
            ++obj_it;
        }
        objects_.clear();

        auto objc_it = complete_objects_.begin();
        while (objc_it != complete_objects_.end())
        {
            delete (objc_it->second);
            ++objc_it;
        }
        complete_objects_.clear();
    }
}

void TypeObjectFactory::create_builtin_annotations()
{
    register_builtin_annotations_types(g_instance);
}

void TypeObjectFactory::nullify_all_entries(const TypeIdentifier* identifier)
{
    for (auto it = identifiers_.begin(); it != identifiers_.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }

    for (auto it = complete_identifiers_.begin(); it != complete_identifiers_.end(); ++it)
    {
        if (it->second == identifier)
        {
            it->second = nullptr;
        }
    }

    auto it = std::find(identifiers_created_.begin(), identifiers_created_.end(), identifier);
    if (it != identifiers_created_.end())
    {
        identifiers_created_.erase(it);
    }
}

const TypeInformation* TypeObjectFactory::get_type_information(
        const std::string &type_name) const
{
    const TypeIdentifier* comp_identifier = get_type_identifier(type_name, true);
    const TypeIdentifier* min_identifier = get_type_identifier(type_name, false);
    if (comp_identifier == nullptr && min_identifier == nullptr)
    {
        return nullptr;
    }

    TypeInformation *information = nullptr;
    if (min_identifier != nullptr)
    {
        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
        auto innerInfo = informations_.find(min_identifier);
        if (innerInfo != informations_.end())
        {
            information = innerInfo->second;
            fill_minimal_information(information, min_identifier);
        }
        else
        {
            information = new TypeInformation();
            fill_minimal_information(information, min_identifier);
            informations_[min_identifier] = information;
            informations_created_.push_back(information);
        }
    }

    if (comp_identifier != nullptr)
    {
        if (information == nullptr)
        {
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(comp_identifier);
            if (innerInfo != informations_.end())
            {
                information = innerInfo->second;
            }
            else
            {
                information = new TypeInformation();
                fill_complete_information(information, comp_identifier);
                informations_[comp_identifier] = information;
                informations_created_.push_back(information);
            }
        }
        else
        {
            fill_complete_information(information, comp_identifier);
        }
    }

    return information;
}

void TypeObjectFactory::fill_minimal_information(
        TypeInformation *info,
        const TypeIdentifier* user_ident) const
{
    const TypeIdentifier* ident = get_stored_type_identifier(user_ident);
    {
        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
        auto it = informations_.find(ident);
        if (it != informations_.end())
        {
            if (info == it->second)
            {
                return;
            }
            info->minimal(it->second->minimal());
            return;
        }
    }

    info->minimal().typeid_with_size().type_id(*ident);
    const TypeObject* obj = get_type_object(ident);

    if (obj == nullptr)
    {
        info->minimal().dependent_typeid_count(0);
        info->minimal().typeid_with_size().typeobject_serialized_size(0);
        // TODO Size in this case should be zero or the size of the identifier?
        // info->minimal().typeid_with_size().typeobject_serialized_size(TypeIdentifier::getCdrSerializedSize(*ident));
    }
    else
    {
        info->minimal().typeid_with_size().typeobject_serialized_size(
            static_cast<uint32_t>(TypeObject::getCdrSerializedSize(*obj)));
    }

    switch(ident->_d())
    {
        /*
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
        case TK_UINT16:
        case TK_UINT32:
        case TK_UINT64:
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_CHAR8:
        case TK_CHAR16:
        case TK_STRING8:
        case TK_STRING16:
            info->minimal().dependent_typeid_count(0);
            break;
        */
        case TK_SEQUENCE:
        {
            info->minimal().dependent_typeid_count(1);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->minimal().sequence_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->minimal().dependent_typeids().push_back(innerInfo->second->minimal().typeid_with_size());
            }
            else
            {
                fill_minimal_dependant_types(info, innerId);
            }
            break;
        }
        case TK_ARRAY:
        {
            info->minimal().dependent_typeid_count(1);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->minimal().array_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->minimal().dependent_typeids().push_back(innerInfo->second->minimal().typeid_with_size());
            }
            else
            {
                fill_minimal_dependant_types(info, innerId);
            }
            break;
        }
        case TK_MAP:
        {
            info->minimal().dependent_typeid_count(2);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->minimal().map_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->minimal().dependent_typeids().push_back(innerInfo->second->minimal().typeid_with_size());
            }
            else
            {
                fill_minimal_dependant_types(info, innerId);
            }
            const TypeIdentifier *keyId = get_stored_type_identifier(
                &obj->minimal().map_type().key().common().type());
            auto keyInfo = informations_.find(keyId);
            if (keyInfo != informations_.end())
            {
                info->minimal().dependent_typeids().push_back(keyInfo->second->minimal().typeid_with_size());
            }
            else
            {
                fill_minimal_dependant_types(info, innerId);
            }
            break;
        }
        case EK_MINIMAL:
            switch(obj->minimal()._d())
            {
                case TK_ALIAS:
                {
                    info->minimal().dependent_typeid_count(1);
                    const TypeIdentifier *innerId = get_stored_type_identifier(
                        &obj->minimal().alias_type().body().common().related_type());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto keyInfo = informations_.find(innerId);
                    if (keyInfo != informations_.end())
                    {
                        info->minimal().dependent_typeids().push_back(keyInfo->second->minimal().typeid_with_size());
                    }
                    else
                    {
                        fill_minimal_dependant_types(info, innerId);
                    }
                    break;
                }
                case TK_STRUCTURE:
                {
                    const MinimalStructMemberSeq& members = obj->minimal().struct_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().member_type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->minimal().dependent_typeids().push_back(
                                memberType->second->minimal().typeid_with_size());
                        }
                        else
                        {
                            fill_minimal_dependant_types(info, innerId);
                        }
                    }
                    info->minimal().dependent_typeid_count(static_cast<int32_t>(members.size()));
                    break;
                }
                case TK_ENUM:
                    // Already fully defined by obj
                    break;
                case TK_BITMASK:
                    // TODO To implement
                    break;
                case TK_BITSET:
                    // TODO To implement
                    break;
                case TK_UNION:
                {
                    const MinimalUnionMemberSeq& members = obj->minimal().union_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->minimal().dependent_typeids().push_back(
                                memberType->second->minimal().typeid_with_size());
                        }
                        else
                        {
                            fill_minimal_dependant_types(info, innerId);
                        }
                    }
                    const TypeIdentifier *descId = get_stored_type_identifier(
                        &obj->minimal().union_type().discriminator().common().type_id());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto descInfo = informations_.find(descId);
                    if (descInfo != informations_.end())
                    {
                        info->minimal().dependent_typeids().push_back(descInfo->second->minimal().typeid_with_size());
                    }
                    else
                    {
                        fill_minimal_dependant_types(info, descId);
                    }
                    info->minimal().dependent_typeid_count(static_cast<int32_t>(members.size() + 1));
                    break;
                }
                case TK_ANNOTATION:
                    // TODO To implement
                    break;
            }
            break;
        case EK_COMPLETE:
            // Cannot happen
            break;
    }
    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
    TypeInformation* new_info = new TypeInformation();
    *new_info = *info;
    informations_[ident] = new_info;
    informations_created_.push_back(new_info);
}

TypeInformation* TypeObjectFactory::get_type_information(
        const TypeIdentifier* identifier) const
{
    const TypeIdentifier* ident = get_stored_type_identifier(identifier);
    {
        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
        auto it = informations_.find(ident);
        if (it != informations_.end())
        {
            return it->second;
        }
    }

    return nullptr;
}

void TypeObjectFactory::fill_complete_dependant_types(
        TypeInformation* info,
        const TypeIdentifier* identifier) const
{
    TypeInformation* information = new TypeInformation();
    fill_complete_information(information, identifier);
    informations_[identifier] = information;
    informations_created_.push_back(information);
    info->complete().dependent_typeids().push_back(information->complete().typeid_with_size());
}

void TypeObjectFactory::fill_minimal_dependant_types(
        TypeInformation* info,
        const TypeIdentifier* identifier) const
{
    TypeInformation* information = new TypeInformation();
    fill_minimal_information(information, identifier);
    informations_[identifier] = information;
    informations_created_.push_back(information);
    info->minimal().dependent_typeids().push_back(information->minimal().typeid_with_size());
}

void TypeObjectFactory::fill_complete_minimal_dependant_types(
        TypeInformation* info,
        const TypeIdentifier* identifier) const
{
    TypeInformation* information = new TypeInformation();
    fill_complete_information(information, identifier);
    informations_[identifier] = information;
    informations_created_.push_back(information);
    info->minimal().dependent_typeids().push_back(information->complete().typeid_with_size());
}

void TypeObjectFactory::fill_complete_information(
        TypeInformation *info,
        const TypeIdentifier* user_ident) const
{
    const TypeIdentifier* ident = get_stored_type_identifier(user_ident);
    {
        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
        auto it = informations_.find(ident);
        if (it != informations_.end())
        {
            if (info == it->second)
            {
                return;
            }
            info->complete(it->second->complete());
            return;
        }
    }

    info->complete().typeid_with_size().type_id(*ident);
    const TypeObject* obj = get_type_object(ident);

    if (obj == nullptr)
    {
        info->complete().dependent_typeid_count(0);
        info->complete().typeid_with_size().typeobject_serialized_size(0);
        // TODO Size in this case should be zero or the size of the identifier?
        // info->complete().typeid_with_size().typeobject_serialized_size(TypeIdentifier::getCdrSerializedSize(*ident));
    }
    else
    {
        info->complete().typeid_with_size().typeobject_serialized_size(
            static_cast<uint32_t>(TypeObject::getCdrSerializedSize(*obj)));
    }

    switch(ident->_d())
    {
        /*
        case TK_BOOLEAN:
        case TK_BYTE:
        case TK_INT16:
        case TK_INT32:
        case TK_INT64:
        case TK_UINT16:
        case TK_UINT32:
        case TK_UINT64:
        case TK_FLOAT32:
        case TK_FLOAT64:
        case TK_FLOAT128:
        case TK_CHAR8:
        case TK_CHAR16:
        case TK_STRING8:
        case TK_STRING16:
            info->complete().dependent_typeid_count(0);
            break;
        */
        case TK_SEQUENCE:
        {
            info->complete().dependent_typeid_count(1);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->complete().sequence_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->complete().dependent_typeids().push_back(innerInfo->second->complete().typeid_with_size());
            }
            else
            {
                fill_complete_dependant_types(info, innerId);
            }
            break;
        }
        case TK_ARRAY:
        {
            info->complete().dependent_typeid_count(1);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->complete().array_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->complete().dependent_typeids().push_back(innerInfo->second->complete().typeid_with_size());
            }
            else
            {
                fill_complete_dependant_types(info, innerId);
            }
            break;
        }
        case TK_MAP:
        {
            info->complete().dependent_typeid_count(2);
            const TypeIdentifier *innerId = get_stored_type_identifier(
                &obj->complete().map_type().element().common().type());
            std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
            auto innerInfo = informations_.find(innerId);
            if (innerInfo != informations_.end())
            {
                info->complete().dependent_typeids().push_back(innerInfo->second->complete().typeid_with_size());
            }
            else
            {
                fill_complete_dependant_types(info, innerId);
            }
            const TypeIdentifier *keyId = get_stored_type_identifier(
                &obj->complete().map_type().key().common().type());
            {
                auto keyInfo = informations_.find(keyId);
                if (keyInfo != informations_.end())
                {
                    info->complete().dependent_typeids().push_back(keyInfo->second->complete().typeid_with_size());
                }
                else
                {
                    fill_complete_dependant_types(info, innerId);
                }
            }
            break;
        }
        case EK_MINIMAL:
            switch(obj->minimal()._d())
            {
                case TK_ALIAS:
                {
                    info->minimal().dependent_typeid_count(1);
                    const TypeIdentifier *innerId = get_stored_type_identifier(
                        &obj->minimal().alias_type().body().common().related_type());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto keyInfo = informations_.find(innerId);
                    if (keyInfo != informations_.end())
                    {
                        info->minimal().dependent_typeids().push_back(keyInfo->second->minimal().typeid_with_size());
                    }
                    else
                    {
                        fill_complete_minimal_dependant_types(info, innerId);
                    }
                    break;
                }
                case TK_STRUCTURE:
                {
                    const MinimalStructMemberSeq& members = obj->minimal().struct_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().member_type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->minimal().dependent_typeids().push_back(
                                memberType->second->minimal().typeid_with_size());
                        }
                        else
                        {
                            fill_complete_minimal_dependant_types(info, innerId);
                        }
                    }
                    info->minimal().dependent_typeid_count(static_cast<int32_t>(members.size()));
                    break;
                }
                case TK_ENUM:
                    // Already fully defined by obj
                    break;
                case TK_BITMASK:
                    // TODO To implement (already fully defined?)
                    break;
                case TK_BITSET:
                    // TODO To implement (already fully defined? Fields are primitives.)
                    break;
                case TK_UNION:
                {
                    const MinimalUnionMemberSeq& members = obj->minimal().union_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->minimal().dependent_typeids().push_back(
                                memberType->second->minimal().typeid_with_size());
                        }
                        else
                        {
                            fill_complete_minimal_dependant_types(info, innerId);
                        }
                    }
                    const TypeIdentifier *descId = get_stored_type_identifier(
                        &obj->minimal().union_type().discriminator().common().type_id());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto descInfo = informations_.find(descId);
                    if (descInfo != informations_.end())
                    {
                        info->minimal().dependent_typeids().push_back(descInfo->second->minimal().typeid_with_size());
                    }
                    else
                    {
                        fill_complete_minimal_dependant_types(info, descId);
                    }
                    info->minimal().dependent_typeid_count(static_cast<int32_t>(members.size() + 1));
                    break;
                }
                case TK_ANNOTATION:
                    // TODO To implement (already fully defined? Fields are primitives.)
                    break;
            }
            break;
        case EK_COMPLETE:
            switch(obj->complete()._d())
            {
                case TK_ALIAS:
                {
                    info->complete().dependent_typeid_count(1);
                    const TypeIdentifier *innerId = get_stored_type_identifier(
                        &obj->complete().alias_type().body().common().related_type());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto keyInfo = informations_.find(innerId);
                    if (keyInfo != informations_.end())
                    {
                        info->complete().dependent_typeids().push_back(keyInfo->second->complete().typeid_with_size());
                    }
                    else
                    {
                        fill_complete_dependant_types(info, innerId);
                    }
                    break;
                }
                case TK_STRUCTURE:
                {
                    const CompleteStructMemberSeq& members = obj->complete().struct_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().member_type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->complete().dependent_typeids().push_back(
                                memberType->second->complete().typeid_with_size());
                        }
                        else
                        {
                            fill_complete_dependant_types(info, innerId);
                        }
                    }
                    info->complete().dependent_typeid_count(static_cast<int32_t>(members.size()));
                    break;
                }
                case TK_ENUM:
                    // Already fully defined by obj
                    break;
                case TK_BITMASK:
                    // TODO To implement
                    break;
                case TK_BITSET:
                    // TODO To implement (already fully defined? Fields are primitives.)
                    break;
                case TK_UNION:
                {
                    const CompleteUnionMemberSeq& members = obj->complete().union_type().member_seq();
                    for (auto member = members.begin(); member != members.end(); ++member)
                    {
                        const TypeIdentifier *innerId = get_stored_type_identifier(
                            &member->common().type_id());
                        std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                        auto memberType = informations_.find(innerId);
                        if (memberType != informations_.end())
                        {
                            info->complete().dependent_typeids().push_back(
                                memberType->second->complete().typeid_with_size());
                        }
                        else
                        {
                            fill_complete_dependant_types(info, innerId);
                        }
                    }
                    const TypeIdentifier *descId = get_stored_type_identifier(
                        &obj->complete().union_type().discriminator().common().type_id());
                    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
                    auto descInfo = informations_.find(descId);
                    if (descInfo != informations_.end())
                    {
                        info->complete().dependent_typeids().push_back(descInfo->second->complete().typeid_with_size());
                    }
                    else
                    {
                        fill_complete_dependant_types(info, descId);
                    }
                    info->complete().dependent_typeid_count(static_cast<int32_t>(members.size() + 1));
                    break;
                }
                case TK_ANNOTATION:
                    // TODO To implement (already fully defined? Fields are primitives.)
                    break;
            }
            break;
    }
    std::lock_guard<std::recursive_mutex> lock(m_MutexInformations);
    TypeInformation* new_info = new TypeInformation();
    *new_info = *info;
    informations_[ident] = new_info;
    informations_created_.push_back(new_info);
}

const TypeObject* TypeObjectFactory::get_type_object(const std::string& type_name, bool complete) const
{
    const TypeIdentifier* identifier = get_type_identifier(type_name, complete);
    if (identifier == nullptr)
    {
        return nullptr;
    }

    return get_type_object(identifier);
}

const TypeObject* TypeObjectFactory::get_type_object(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexObjects);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        if (complete_objects_.find(identifier) != complete_objects_.end())
        {
            return complete_objects_.at(identifier);
        }
    }
    else
    {
        if (objects_.find(identifier) != objects_.end())
        {
            return objects_.at(identifier);
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = get_stored_type_identifier(identifier);
    if (internalId != nullptr)
    {
        if (internalId == identifier)
        {
            return nullptr; // Type without object
        }
        return get_type_object(internalId);
    }

    return nullptr;
}

TypeKind TypeObjectFactory::get_type_kind(const std::string& type_name) const
{
    if (type_name == TKNAME_BOOLEAN)
    {
        return TK_BOOLEAN;
    }
    else if (type_name == TKNAME_INT16)
    {
        return TK_INT16;
    }
    else if (type_name == TKNAME_INT32)
    {
        return TK_INT32;
    }
    else if (type_name == TKNAME_UINT16)
    {
        return TK_UINT16;
    }
    else if (type_name == TKNAME_UINT32)
    {
        return TK_UINT32;
    }
    else if (type_name == TKNAME_FLOAT32)
    {
        return TK_FLOAT32;
    }
    else if (type_name == TKNAME_FLOAT64)
    {
        return TK_FLOAT64;
    }
    else if (type_name == TKNAME_CHAR8)
    {
        return TK_CHAR8;
    }
    else if (type_name == TKNAME_BYTE || type_name == TKNAME_INT8 || type_name == TKNAME_UINT8)
    {
        return TK_BYTE;
    }
    else if (type_name.find("strings_") == 0)
    {
        return TI_STRING8_SMALL;
    }
    else if (type_name.find("stringl_") == 0)
    {
        return TI_STRING8_LARGE;
    }
    else if (type_name.find("sets_") == 0)
    {
        return TI_PLAIN_SEQUENCE_SMALL;
    }
    else if (type_name.find("setl_") == 0)
    {
        return TI_PLAIN_SEQUENCE_LARGE;
    }
    else if (type_name.find("arrays_") == 0)
    {
        return TI_PLAIN_ARRAY_SMALL;
    }
    else if (type_name.find("arrayl_") == 0)
    {
        return TI_PLAIN_ARRAY_LARGE;
    }
    else if (type_name == TKNAME_INT64)
    {
        return TK_INT64;
    }
    else if (type_name == TKNAME_UINT64)
    {
        return TK_UINT64;
    }
    else if (type_name == TKNAME_FLOAT128)
    {
        return TK_FLOAT128;
    }
    else if (type_name == TKNAME_CHAR16 || type_name == TKNAME_CHAR16T)
    {
        return TK_CHAR16;
    }
    else if (type_name.find("wstrings_") == 0)
    {
        return TI_STRING16_SMALL;
    }
    else if (type_name.find("wstringl_") == 0)
    {
        return TI_STRING16_LARGE;
    }
    else if (type_name.find("sequences_") == 0)
    {
        return TI_PLAIN_SEQUENCE_SMALL;
    }
    else if (type_name.find("sequencel_") == 0)
    {
        return TI_PLAIN_SEQUENCE_LARGE;
    }
    else if (type_name.find("maps_") == 0)
    {
        return TI_PLAIN_MAP_SMALL;
    }
    else if (type_name.find("mapl_") == 0)
    {
        return TI_PLAIN_MAP_LARGE;
    }
    else if (get_type_identifier(type_name) != nullptr)
    {
        return EK_MINIMAL;
    }
    else
    {
        return TK_NONE;
    }
}

std::string TypeObjectFactory::get_type_name(const TypeKind kind) const
{
    switch (kind)
    {
        // Primitive types, already defined (never will be asked, but ok)
        case TK_BOOLEAN: return TKNAME_BOOLEAN;
        case TK_INT16: return TKNAME_INT16;
        case TK_INT32: return TKNAME_INT32;
        case TK_UINT16: return TKNAME_UINT16;
        case TK_UINT32: return TKNAME_UINT32;
        case TK_FLOAT32: return TKNAME_FLOAT32;
        case TK_FLOAT64: return TKNAME_FLOAT64;
        case TK_CHAR8: return TKNAME_CHAR8;
        case TK_BYTE: return TKNAME_BYTE;
        case TK_INT64: return TKNAME_INT64;
        case TK_UINT64: return TKNAME_UINT64;
        case TK_FLOAT128: return TKNAME_FLOAT128;
        case TK_CHAR16: return TKNAME_CHAR16;
        default:
            break;
    }
    return "";
}

const TypeIdentifier* TypeObjectFactory::get_primitive_type_identifier(TypeKind kind) const
{
    std::string typeName = get_type_name(kind);
    if (typeName.empty()) return nullptr;
    return get_type_identifier(typeName);
}

/*
const TypeIdentifier* TypeObjectFactory::TryCreateTypeIdentifier(const std::string& type_name)
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    // TODO Makes sense here? I don't think so.
}
*/

const TypeIdentifier* TypeObjectFactory::get_type_identifier(const std::string& type_name, bool complete) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (complete)
    {
        if (complete_identifiers_.find(type_name) != complete_identifiers_.end())
        {
            return complete_identifiers_.at(type_name);
        }
        /*else // Try it with minimal
        {
            return get_type_identifier(type_name, false);
        }*/
    }
    else
    {
        if (identifiers_.find(type_name) != identifiers_.end())
        {
            return identifiers_.at(type_name);
        }
    }

    // Try with aliases
    if (aliases_.find(type_name) != aliases_.end())
    {
        return get_type_identifier(aliases_.at(type_name), complete);
    }

    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_type_identifier_trying_complete(const std::string& type_name) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);

    if (complete_identifiers_.find(type_name) != complete_identifiers_.end())
    {
        return complete_identifiers_.at(type_name);
    }
    else // Try it with minimal
    {
        return get_type_identifier(type_name, false);
    }
}

const TypeIdentifier* TypeObjectFactory::get_stored_type_identifier(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return nullptr;
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : complete_identifiers_)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    else
    {
        for (auto& it : identifiers_)
        {
            if (*(it.second) == *identifier) return it.second;
        }
    }
    // If isn't minimal, return directly
    if (identifier->_d() < EK_MINIMAL)
    {
        return identifier;
    }
    return nullptr;
}

std::string TypeObjectFactory::get_type_name(const TypeIdentifier* identifier) const
{
    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    if (identifier == nullptr) return "<NULLPTR>";
    if (identifier->_d() == EK_COMPLETE)
    {
        for (auto& it : complete_identifiers_)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }
    else
    {
        for (auto& it : identifiers_)
        {
            if (*(it.second) == *identifier) return it.first;
        }
    }

    // Maybe they are using an external TypeIdentifier?
    const TypeIdentifier* internalId = get_stored_type_identifier(identifier);
    if (internalId == identifier)
    {
        // If the execution reached this point, a lesser than minimal no stored identifier was provided.
        // Calculate the name and store it.
        return generate_name_and_store_type_identifier(identifier);
    }
    else if (internalId != nullptr)
    {
        return get_type_name(internalId);
    }

    return "UNDEF";
}

std::string TypeObjectFactory::generate_name_and_store_type_identifier(
        const TypeIdentifier* identifier) const
{
    if (identifier->_d() < EK_MINIMAL)
    {
        switch (identifier->_d())
        {
            case TI_PLAIN_ARRAY_SMALL:
            {
                std::vector<uint32_t> bounds;
                for (SBound sb : identifier->array_sdefn().array_bound_seq())
                {
                    bounds.push_back(sb);
                }
                return TypeNamesGenerator::get_array_type_name(
                    get_type_name(identifier->array_sdefn().element_identifier()),
                    bounds,
                    true);
            }
            case TI_PLAIN_ARRAY_LARGE:
            {
                return TypeNamesGenerator::get_array_type_name(
                    get_type_name(identifier->array_ldefn().element_identifier()),
                    identifier->array_ldefn().array_bound_seq(),
                    true);
            }
            case TI_PLAIN_SEQUENCE_SMALL:
            {
                return TypeNamesGenerator::get_sequence_type_name(
                    get_type_name(identifier->seq_sdefn().element_identifier()),
                    identifier->seq_sdefn().bound(),
                    true);
            }
            case TI_PLAIN_SEQUENCE_LARGE:
            {
                return TypeNamesGenerator::get_sequence_type_name(
                    get_type_name(identifier->seq_ldefn().element_identifier()),
                    identifier->seq_ldefn().bound(),
                    true);
            }
            case TI_STRING8_SMALL:
            {
                return TypeNamesGenerator::get_string_type_name(
                    identifier->string_sdefn().bound(),
                    false,
                    true);
            }
            case TI_STRING8_LARGE:
            {
                return TypeNamesGenerator::get_string_type_name(
                    identifier->string_ldefn().bound(),
                    false,
                    true);
            }
            case TI_STRING16_SMALL:
            {
                return TypeNamesGenerator::get_string_type_name(
                    identifier->string_sdefn().bound(),
                    true,
                    true);
            }
            case TI_STRING16_LARGE:
            {
                return TypeNamesGenerator::get_string_type_name(
                    identifier->string_ldefn().bound(),
                    true,
                    true);
            }
            case TI_PLAIN_MAP_SMALL:
            {
                return TypeNamesGenerator::get_map_type_name(
                    get_type_name(identifier->map_sdefn().key_identifier()),
                    get_type_name(identifier->map_sdefn().element_identifier()),
                    identifier->map_sdefn().bound(),
                    true);
            }
            case TI_PLAIN_MAP_LARGE:
            {
                return TypeNamesGenerator::get_map_type_name(
                    get_type_name(identifier->map_ldefn().key_identifier()),
                    get_type_name(identifier->map_ldefn().element_identifier()),
                    identifier->map_ldefn().bound(),
                    true);
            }
            case TI_STRONGLY_CONNECTED_COMPONENT: // TODO: Not yet supported.
            default:
                return "UNDEF";
        }
    }
    return "UNDEF";
}

const TypeIdentifier* TypeObjectFactory::try_get_complete(const TypeIdentifier* identifier) const
{
    if (identifier->_d() == EK_COMPLETE)
    {
        return identifier;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    std::string name = get_type_name(identifier);
    return get_type_identifier_trying_complete(name);
}

bool TypeObjectFactory::is_type_identifier_complete(
        const TypeIdentifier* identifier) const
{
    switch(identifier->_d())
    {
        case TI_STRING8_SMALL:
        case TI_STRING8_LARGE:
        case TI_STRING16_SMALL:
        case TI_STRING16_LARGE:
            return false;
        case TI_PLAIN_SEQUENCE_SMALL:
            return is_type_identifier_complete(identifier->seq_sdefn().element_identifier());
        case TI_PLAIN_SEQUENCE_LARGE:
            return is_type_identifier_complete(identifier->seq_ldefn().element_identifier());
        case TI_PLAIN_ARRAY_SMALL:
            return is_type_identifier_complete(identifier->array_sdefn().element_identifier());
        case TI_PLAIN_ARRAY_LARGE:
            return is_type_identifier_complete(identifier->array_ldefn().element_identifier());
        case TI_PLAIN_MAP_SMALL:
            return is_type_identifier_complete(identifier->map_sdefn().element_identifier())
                   && is_type_identifier_complete(identifier->map_sdefn().key_identifier());
        case TI_PLAIN_MAP_LARGE:
            return is_type_identifier_complete(identifier->map_ldefn().element_identifier())
                   && is_type_identifier_complete(identifier->map_ldefn().key_identifier());
        case TI_STRONGLY_CONNECTED_COMPONENT:
            return false;
        case EK_COMPLETE:
            return true;
        case EK_MINIMAL:
            return false;
        default:
            return false;
    }
}

void TypeObjectFactory::add_type_identifier(const std::string& type_name, const TypeIdentifier* identifier)
{
    const TypeIdentifier* alreadyExists = get_stored_type_identifier(identifier);
    if (alreadyExists != nullptr && alreadyExists != identifier)
    {
        // Don't copy
        if (is_type_identifier_complete(alreadyExists))
        {
            complete_identifiers_[type_name] = alreadyExists;
        }
        else
        {
            identifiers_[type_name] = alreadyExists;
        }
        return;
    }

    std::unique_lock<std::recursive_mutex> scoped(m_MutexIdentifiers);
    //identifiers_.insert(std::pair<const std::string, const TypeIdentifier*>(type_name, identifier));
    if (is_type_identifier_complete(identifier))
    {
        if (complete_identifiers_.find(type_name) == complete_identifiers_.end())
        {
            TypeIdentifier* id = new TypeIdentifier();
            identifiers_created_.push_back(id);
            *id = *identifier;
            complete_identifiers_[type_name] = id;
        }
    }
    else
    {
        if (identifiers_.find(type_name) == identifiers_.end())
        {
            TypeIdentifier* id = new TypeIdentifier();
            identifiers_created_.push_back(id);
            *id = *identifier;
            identifiers_[type_name] = id;
        }
    }
}

void TypeObjectFactory::add_type_object(const std::string& type_name, const TypeIdentifier* identifier,
    const TypeObject* object)
{
    add_type_identifier(type_name, identifier);

    std::unique_lock<std::recursive_mutex> scopedObj(m_MutexObjects);

    if (object != nullptr)
    {
        if (identifier->_d() >= EK_MINIMAL)
        {
            if (object->_d() == EK_MINIMAL)
            {
                const TypeIdentifier* typeId = identifiers_[type_name];
                if (objects_.find(typeId) == objects_.end())
                {
                    TypeObject* obj = new TypeObject();
                    *obj = *object;
                    objects_[typeId] = obj;
                }
            }
            else if (object->_d() == EK_COMPLETE)
            {
                const TypeIdentifier* typeId = complete_identifiers_[type_name];
                if (complete_objects_.find(typeId) == complete_objects_.end())
                {
                    TypeObject* obj = new TypeObject();
                    *obj = *object;
                    complete_objects_[typeId] = obj;
                }
            }
        }
        else
        {
            const TypeIdentifier* typeId = identifiers_[type_name];
            if (object->_d() == EK_MINIMAL)
            {
                if (objects_.find(typeId) == objects_.end())
                {
                    TypeObject* obj = new TypeObject();
                    *obj = *object;
                    objects_[typeId] = obj;
                }
            }
            else if (object->_d() == EK_COMPLETE)
            {
                if (complete_objects_.find(typeId) == complete_objects_.end())
                {
                    TypeObject* obj = new TypeObject();
                    *obj = *object;
                    complete_objects_[typeId] = obj;
                }
            }
        }
    }
}

const TypeIdentifier* TypeObjectFactory::get_string_identifier(
        uint32_t bound,
        bool wide)
{
    std::string type = TypeNamesGenerator::get_string_type_name(bound, wide, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(type);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(wide ? TI_STRING16_SMALL : TI_STRING8_SMALL);
            auxIdent.string_sdefn().bound(static_cast<octet>(bound));
        }
        else
        {
            auxIdent._d(wide ? TI_STRING16_LARGE : TI_STRING8_LARGE);
            auxIdent.string_ldefn().bound(bound);
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(type, auxIdent));
        //identifiers_[type] = auxIdent;
        add_type_identifier(type, &auxIdent);
        return get_type_identifier(type);
    }
    return nullptr;
}

const TypeIdentifier* TypeObjectFactory::get_sequence_identifier(const std::string& type_name,
    uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::get_sequence_type_name(type_name, bound, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? get_type_identifier_trying_complete(type_name)
            : get_type_identifier(type_name);

        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(TI_PLAIN_SEQUENCE_SMALL);
            auxIdent.seq_sdefn().bound(static_cast<octet>(bound));
            auxIdent.seq_sdefn().element_identifier(innerIdent);
            auxIdent.seq_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.seq_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.seq_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.seq_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.seq_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.seq_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.seq_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.seq_sdefn().header().equiv_kind(get_type_kind(type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_SEQUENCE_LARGE);
            auxIdent.seq_ldefn().bound(bound);
            auxIdent.seq_ldefn().element_identifier(innerIdent);
            auxIdent.seq_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.seq_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.seq_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.seq_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.seq_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.seq_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.seq_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.seq_ldefn().header().equiv_kind(get_type_kind(type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        if (complete)
        {
            return get_type_identifier_trying_complete(auxType);
        }
        else
        {
            return get_type_identifier(auxType);
        }
    }
}

const TypeIdentifier* TypeObjectFactory::get_array_identifier(const std::string& type_name,
    const std::vector<uint32_t> &bound, bool complete)
{
    uint32_t size;
    std::string auxType = TypeNamesGenerator::get_array_type_name(type_name, bound, size, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* innerIdent = (complete)
            ? get_type_identifier_trying_complete(type_name)
            : get_type_identifier(type_name);

        TypeIdentifier auxIdent;
        if (size < 256)
        {
            auxIdent._d(TI_PLAIN_ARRAY_SMALL);
            for (uint32_t b : bound)
            {
                auxIdent.array_sdefn().array_bound_seq().push_back(static_cast<octet>(b));
            }
            auxIdent.array_sdefn().element_identifier(innerIdent);
            auxIdent.array_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.array_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.array_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.array_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.array_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.array_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.array_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.array_sdefn().header().equiv_kind(get_type_kind(type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_ARRAY_LARGE);
            for (uint32_t b : bound)
            {
                auxIdent.array_ldefn().array_bound_seq().push_back(b);
            }
            auxIdent.array_ldefn().element_identifier(innerIdent);
            auxIdent.array_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.array_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.array_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.array_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.array_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.array_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.array_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.array_ldefn().header().equiv_kind(get_type_kind(type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        if (complete)
        {
            return get_type_identifier_trying_complete(auxType);
        }
        else
        {
            return get_type_identifier(auxType);
        }
    }
}

const TypeIdentifier* TypeObjectFactory::get_map_identifier(const std::string& key_type_name,
    const std::string& value_type_name, uint32_t bound, bool complete)
{
    std::string auxType = TypeNamesGenerator::get_map_type_name(key_type_name, value_type_name, bound, false);

    const TypeIdentifier* c_auxIdent = get_type_identifier(auxType, complete);

    if (c_auxIdent != nullptr)
    {
        return c_auxIdent;
    }
    else
    {
        const TypeIdentifier* keyIdent = (complete)
            ? get_type_identifier_trying_complete(key_type_name)
            : get_type_identifier(key_type_name);
        const TypeIdentifier* valIdent = (complete)
            ? get_type_identifier_trying_complete(value_type_name)
            : get_type_identifier(value_type_name);

        TypeIdentifier auxIdent;
        if (bound < 256)
        {
            auxIdent._d(TI_PLAIN_MAP_SMALL);
            auxIdent.map_sdefn().bound(static_cast<octet>(bound));
            auxIdent.map_sdefn().element_identifier(valIdent);
            auxIdent.map_sdefn().key_identifier(keyIdent);
            auxIdent.map_sdefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_sdefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_sdefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.map_sdefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.map_sdefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_sdefn().header().element_flags().IS_KEY(false);
            auxIdent.map_sdefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.map_sdefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_sdefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_sdefn().key_flags().IS_EXTERNAL(false);
            auxIdent.map_sdefn().key_flags().IS_OPTIONAL(false);
            auxIdent.map_sdefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_sdefn().key_flags().IS_KEY(false);
            auxIdent.map_sdefn().key_flags().IS_DEFAULT(false);
            auxIdent.map_sdefn().header().equiv_kind(get_type_kind(value_type_name));
        }
        else
        {
            auxIdent._d(TI_PLAIN_MAP_LARGE);
            auxIdent.map_ldefn().bound(bound);
            auxIdent.map_ldefn().element_identifier(valIdent);
            auxIdent.map_ldefn().key_identifier(keyIdent);
            auxIdent.map_ldefn().header().element_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_ldefn().header().element_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_ldefn().header().element_flags().IS_EXTERNAL(false);
            auxIdent.map_ldefn().header().element_flags().IS_OPTIONAL(false);
            auxIdent.map_ldefn().header().element_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_ldefn().header().element_flags().IS_KEY(false);
            auxIdent.map_ldefn().header().element_flags().IS_DEFAULT(false);
            auxIdent.map_ldefn().key_flags().TRY_CONSTRUCT1(false);
            auxIdent.map_ldefn().key_flags().TRY_CONSTRUCT2(false);
            auxIdent.map_ldefn().key_flags().IS_EXTERNAL(false);
            auxIdent.map_ldefn().key_flags().IS_OPTIONAL(false);
            auxIdent.map_ldefn().key_flags().IS_MUST_UNDERSTAND(false);
            auxIdent.map_ldefn().key_flags().IS_KEY(false);
            auxIdent.map_ldefn().key_flags().IS_DEFAULT(false);
            auxIdent.map_ldefn().header().equiv_kind(get_type_kind(value_type_name));
        }
        //identifiers_.insert(std::pair<std::string, TypeIdentifier*>(auxType, auxIdent));
        //identifiers_[auxType] = auxIdent;
        add_type_identifier(auxType, &auxIdent);
        if (complete)
        {
            return get_type_identifier_trying_complete(auxType);
        }
        else
        {
            return get_type_identifier(auxType);
        }
    }
}

static TypeKind GetTypeKindFromIdentifier(const TypeIdentifier* identifier)
{
    if (identifier == nullptr) return TK_NONE;
    switch (identifier->_d())
    {
        case TI_STRING8_SMALL:
        case TI_STRING8_LARGE:
            return TK_STRING8;
        case TI_STRING16_SMALL:
        case TI_STRING16_LARGE:
            return TK_STRING16;
        case TI_PLAIN_SEQUENCE_SMALL:
        case TI_PLAIN_SEQUENCE_LARGE:
            return TK_SEQUENCE;
        case TI_PLAIN_ARRAY_SMALL:
        case TI_PLAIN_ARRAY_LARGE:
            return TK_ARRAY;
        case TI_PLAIN_MAP_SMALL:
        case TI_PLAIN_MAP_LARGE:
            return TK_MAP;
        case TI_STRONGLY_CONNECTED_COMPONENT:
            return TK_NONE;
        case EK_COMPLETE:
        case EK_MINIMAL:
        default:
            return identifier->_d();
    }
}

DynamicType_ptr TypeObjectFactory::build_dynamic_type(const std::string& name, const TypeIdentifier* identifier,
    const TypeObject* object) const
{
    TypeKind kind = GetTypeKindFromIdentifier(identifier);
    TypeDescriptor descriptor(name, kind);
    switch (kind)
    {
    // Basic types goes as default!
    /*
    case TK_NONE:
    case TK_BOOLEAN:
    case TK_BYTE:
    case TK_INT16:
    case TK_INT32:
    case TK_INT64:
    case TK_UINT16:
    case TK_UINT32:
    case TK_UINT64:
    case TK_FLOAT32:
    case TK_FLOAT64:
    case TK_FLOAT128:
    case TK_CHAR8:
    case TK_CHAR16:
        break;
    */
        case TK_STRING8:
        {
            if (identifier->_d() == TI_STRING8_SMALL)
            {
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.bound_.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.element_type_ = DynamicTypeBuilderFactory::get_instance()->create_char8_type();
            break;
        }
        case TK_STRING16:
        {
            if (identifier->_d() == TI_STRING16_SMALL)
            {
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->string_sdefn().bound()));
            }
            else
            {
                descriptor.bound_.emplace_back(identifier->string_ldefn().bound());
            }
            descriptor.element_type_ = DynamicTypeBuilderFactory::get_instance()->create_char16_type();
            break;
        }
        case TK_SEQUENCE:
        {
            if (identifier->_d() == TI_PLAIN_SEQUENCE_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->seq_sdefn().element_identifier());
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->seq_sdefn().bound()));
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            else
            {
                const TypeIdentifier* aux = try_get_complete(identifier->seq_ldefn().element_identifier());
                descriptor.bound_.emplace_back(identifier->seq_ldefn().bound());
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            break;
        }
        case TK_ARRAY:
        {
            if (identifier->_d() == TI_PLAIN_ARRAY_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->array_sdefn().element_identifier());
                for (octet b : identifier->array_sdefn().array_bound_seq())
                {
                    descriptor.bound_.emplace_back(static_cast<uint32_t>(b));
                }
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            else
            {
                const TypeIdentifier* aux = identifier->array_ldefn().element_identifier();
                descriptor.bound_ = identifier->array_ldefn().array_bound_seq();
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }
            break;
        }
        case TK_MAP:
        {
            if (identifier->_d() == TI_PLAIN_MAP_SMALL)
            {
                const TypeIdentifier* aux = try_get_complete(identifier->map_sdefn().element_identifier());
                const TypeIdentifier* aux2 = try_get_complete(identifier->map_sdefn().key_identifier());
                descriptor.bound_.emplace_back(static_cast<uint32_t>(identifier->map_sdefn().bound()));
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
                descriptor.key_element_type_ = build_dynamic_type(get_type_name(aux), aux2, get_type_object(aux2));
            }
            else
            {
                const TypeIdentifier* aux = try_get_complete(identifier->map_ldefn().element_identifier());
                const TypeIdentifier* aux2 = try_get_complete(identifier->map_ldefn().key_identifier());
                descriptor.bound_.emplace_back(identifier->map_ldefn().bound());
                descriptor.element_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
                descriptor.key_element_type_ = build_dynamic_type(get_type_name(aux), aux2, get_type_object(aux2));
            }
            break;
        }
        case EK_MINIMAL:
        case EK_COMPLETE:
            // A MinimalTypeObject cannot instantiate a valid TypeDescriptor, but maybe the object isn't minimal
            if (object != nullptr && object->_d() == EK_COMPLETE)
            {
                return build_dynamic_type(descriptor, object);
            }
            break;
        case TK_NONE:
            return DynamicType_ptr(nullptr); // Maybe in discovery, return nullptr quietly.
        default:
            break;
    }

    DynamicTypeBuilder_ptr outputType = DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);
    //outputType->set_name(name);
    if (outputType != nullptr)
    {
        return outputType->build();
    }
    return DynamicType_ptr(nullptr);
}

// TODO annotations
DynamicType_ptr TypeObjectFactory::build_dynamic_type(
        TypeDescriptor& descriptor,
        const TypeObject* object,
        const DynamicType_ptr annotation_member_type) const
{
    if (object == nullptr || object->_d() != EK_COMPLETE)
    {
        return DynamicType_ptr(nullptr);
    }

    // Change descriptor's kind
    descriptor.set_kind(object->complete()._d());

    switch (object->complete()._d())
    {
        // From here, we need TypeObject
        case TK_ALIAS:
        {
            const TypeIdentifier* aux =
                get_stored_type_identifier(&object->complete().alias_type().body().common().related_type());
            descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            descriptor.set_name(object->complete().alias_type().header().detail().type_name());
            DynamicTypeBuilder_ptr alias_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(alias_type, object->complete().alias_type().header().detail().ann_custom());

            return alias_type->build();
        }
        case TK_STRUCTURE:
        {
            const TypeIdentifier* aux = &object->complete().struct_type().header().base_type();
            if (aux->_d() == EK_COMPLETE)
            {
                descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }

            DynamicTypeBuilder_ptr struct_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(struct_type, object->complete().struct_type().header().detail().ann_custom());

            //uint32_t order = 0;
            const CompleteStructMemberSeq& structVector = object->complete().struct_type().member_seq();
            for (auto member = structVector.begin(); member != structVector.end(); ++member)
            {
                //const TypeIdentifier* auxMem = &member.common().member_type_id();
                const TypeIdentifier* auxMem = get_stored_type_identifier(&member->common().member_type_id());
                if (auxMem == nullptr)
                {
                    logWarning(DYNAMIC_TYPES, "(Struct) auxMem is nullptr, but original member has "
                        << (int)member->common().member_type_id()._d());
                }
                MemberDescriptor memDesc;
                memDesc.id_ = member->common().member_id();
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                //memDesc.set_index(order++);
                memDesc.set_name(member->detail().name());
                struct_type->add_member(&memDesc);
                apply_member_annotations(struct_type, member->common().member_id(), member->detail().ann_custom());
            }
            return struct_type->build();
        }
        case TK_ENUM:
        {
            // bit_bound annotation effect!
            descriptor.annotation_set_bit_bound(object->complete().enumerated_type().header().common().bit_bound());

            DynamicTypeBuilder_ptr enum_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(enum_type, object->complete().enumerated_type().header().detail().ann_custom());
            /*
            {
                const AppliedAnnotationSeq& annotations =
                    object->complete().enumerated_type().header().detail().ann_custom();
                for (const AppliedAnnotation& annotation : annotations)
                {
                    const TypeIdentifier* anno_id = get_stored_type_identifier(&annotation.annotation_typeid());
                    if (anno_id == nullptr)
                    {
                        logWarning(DYNAMIC_TYPES, "(Annotation) anno_id is nullptr, but original member has "
                            << (int)annotation.annotation_typeid()._d());
                    }
                    AnnotationDescriptor anno_desc;
                    anno_desc.set_type(build_dynamic_type(get_type_name(anno_id), anno_id, get_type_object(anno_id)));
                    const AppliedAnnotationParameterSeq& anno_params = annotation.param_seq();
                    for (const AppliedAnnotationParameter a_param : anno_params)
                    {
                        std::string param_key = get_key_from_hash(anno_desc.type(), a_param.paramname_hash());
                        anno_desc.set_value(param_key, a_param.value().to_string());
                    }
                    enum_type->apply_annotation(anno_desc);
                }
            }
            */

            const CompleteEnumeratedLiteralSeq& enumVector = object->complete().enumerated_type().literal_seq();
            for (auto member = enumVector.begin(); member != enumVector.end(); ++member)
            {
                enum_type->add_empty_member(member->common().value(), member->detail().name());
                apply_member_annotations(enum_type, member->common().value(), member->detail().ann_custom());
                if (member->common().flags().IS_DEFAULT())
                {
                    AnnotationDescriptor def_flag;
                    def_flag.set_value(ANNOTATION_DEFAULT_LITERAL_ID, CONST_TRUE);
                    enum_type->apply_annotation_to_member(member->common().value(), def_flag);
                }
            }
            return enum_type->build();
        }
        case TK_BITMASK:
        {
            descriptor.annotation_set_bit_bound(object->complete().bitmask_type().header().common().bit_bound());
            descriptor.bound_.emplace_back(static_cast<uint32_t>(
                object->complete().bitmask_type().header().common().bit_bound()));
            descriptor.element_type_ = DynamicTypeBuilderFactory::get_instance()->create_bool_type();

            DynamicTypeBuilder_ptr bitmask_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(bitmask_type, object->complete().bitmask_type().header().detail().ann_custom());

            const CompleteBitflagSeq& seq = object->complete().bitmask_type().flag_seq();
            for (auto member = seq.begin(); member != seq.end(); ++member)
            {
                bitmask_type->add_empty_member(member->common().position(), member->detail().name());
                MemberId m_id = bitmask_type->get_member_id_by_name(member->detail().name());
                // member->common().position() should be already an annotation
                apply_member_annotations(bitmask_type, m_id, member->detail().ann_custom());
            }
            return bitmask_type->build();
        }
        case TK_BITSET:
        {
            const TypeIdentifier* aux = &object->complete().bitset_type().header().base_type();
            if (aux->_d() == EK_COMPLETE)
            {
                descriptor.base_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));
            }

            DynamicTypeBuilder_ptr bitsetType =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(bitsetType, object->complete().bitset_type().header().detail().ann_custom());

            //uint32_t order = 0;
            const CompleteBitfieldSeq& fields = object->complete().bitset_type().field_seq();
            for (auto member = fields.begin(); member != fields.end(); ++member)
            {
                //const TypeIdentifier* auxMem = &member.common().member_type_id();
                const TypeIdentifier* auxMem = get_primitive_type_identifier(member->common().holder_type());
                if (auxMem == nullptr)
                {
                    logWarning(DYNAMIC_TYPES, "(Bitset) auxMem is nullptr, but original member has "
                        << (int)member->common().holder_type());
                }
                MemberDescriptor memDesc;
                //memDesc.id_ = order++;
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                memDesc.set_name(member->detail().name());
                // bounds are meant for string, arrays, sequences, maps, but not for bitset!
                // Lack in the standard?
                bitsetType->add_member(&memDesc);
                MemberId m_id = bitsetType->get_member_id_by_name(memDesc.get_name());
                // member->common().position() and member->common().bitcount() should be annotations
                apply_member_annotations(bitsetType, m_id, member->detail().ann_custom());
            }
            return bitsetType->build();

            //logError(XTYPES, "Bitset isn't supported by DynamicType");
            //return nullptr;
        }
        case TK_UNION:
        {
            const TypeIdentifier* aux =
                get_stored_type_identifier(&object->complete().union_type().discriminator().common().type_id());
            descriptor.discriminator_type_ = build_dynamic_type(get_type_name(aux), aux, get_type_object(aux));

            DynamicTypeBuilder_ptr union_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            // Apply type's annotations
            apply_type_annotations(union_type, object->complete().union_type().header().detail().ann_custom());

            //uint32_t order = 0;
            const CompleteUnionMemberSeq& unionVector = object->complete().union_type().member_seq();
            for (auto member = unionVector.begin(); member != unionVector.end(); ++member)
            {
                const TypeIdentifier* auxMem = get_stored_type_identifier(&member->common().type_id());
                if (auxMem == nullptr)
                {
                    logWarning(DYNAMIC_TYPES, "(Union) auxMem is nullptr, but original member has "
                        << (int)member->common().type_id()._d());
                }
                MemberDescriptor memDesc;
                memDesc.set_type(build_dynamic_type(get_type_name(auxMem), auxMem, get_type_object(auxMem)));
                //memDesc.set_index(order++);
                memDesc.id_ = member->common().member_id();
                memDesc.set_name(member->detail().name());
                memDesc.set_default_union_value(member->common().member_flags().IS_DEFAULT());
                if (descriptor.discriminator_type_->get_kind() == TK_ENUM)
                {
                    DynamicTypeMember enumMember;
                    descriptor.discriminator_type_->get_member(enumMember, memDesc.id_);
                    memDesc.default_value_ = enumMember.get_descriptor()->name_;
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.add_union_case_index(lab);
                    }
                }
                else
                {
                    memDesc.default_value_ = std::to_string(memDesc.id_);
                    for (uint32_t lab : member->common().label_seq())
                    {
                        memDesc.add_union_case_index(lab);
                    }
                }
                union_type->add_member(&memDesc);
                apply_member_annotations(union_type, member->common().member_id(), member->detail().ann_custom());
            }

            return union_type->build();
        }
        case TK_ANNOTATION:
        {
            DynamicTypeBuilder_ptr annotation_type =
                DynamicTypeBuilderFactory::get_instance()->create_custom_builder(&descriptor);

            for (const CompleteAnnotationParameter& member : object->complete().annotation_type().member_seq())
            {
                const TypeIdentifier* aux_mem = get_stored_type_identifier(&member.common().member_type_id());
                if (aux_mem == nullptr)
                {
                    logWarning(DYNAMIC_TYPES, "(Annotation) aux_mem is nullptr, but original member has "
                        << (int)member.common().member_type_id()._d());
                }

                MemberDescriptor mem_desc;
                mem_desc.set_name(member.name());
                if (annotation_member_type != nullptr)
                {
                    mem_desc.set_type(annotation_member_type);
                }
                else
                {
                    mem_desc.set_type(build_dynamic_type(get_type_name(aux_mem), aux_mem, get_type_object(aux_mem)));
                }
                mem_desc.set_default_value(member.default_value().to_string());
                annotation_type->add_member(&mem_desc);
            }
            // Annotation inner definitions?

            return annotation_type->build();
        }
        default:
            break;
    }
    return DynamicType_ptr(nullptr);
}

void TypeObjectFactory::apply_type_annotations(
        DynamicTypeBuilder_ptr& type_builder,
        const AppliedAnnotationSeq& annotations) const
{
    for (const AppliedAnnotation& annotation : annotations)
    {
        const TypeIdentifier* anno_id = get_stored_type_identifier(&annotation.annotation_typeid());
        if (anno_id == nullptr)
        {
            logWarning(DYNAMIC_TYPES, "(Annotation) anno_id is nullptr, but original member has "
                << (int)annotation.annotation_typeid()._d());
        }
        AnnotationDescriptor anno_desc;
        anno_desc.set_type(build_dynamic_type(get_type_name(anno_id), anno_id, get_type_object(anno_id)));
        const AppliedAnnotationParameterSeq& anno_params = annotation.param_seq();
        for (const AppliedAnnotationParameter& a_param : anno_params)
        {
            std::string param_key = get_key_from_hash(anno_desc.type(), a_param.paramname_hash());
            anno_desc.set_value(param_key, a_param.value().to_string());
        }
        type_builder->apply_annotation(anno_desc);
    }
}

void TypeObjectFactory::apply_member_annotations(
        DynamicTypeBuilder_ptr& parent_type_builder,
        MemberId member_id,
        const AppliedAnnotationSeq& annotations) const
{
    for (const AppliedAnnotation& annotation : annotations)
    {
        const TypeIdentifier* anno_id = get_stored_type_identifier(&annotation.annotation_typeid());
        if (anno_id == nullptr)
        {
            logWarning(DYNAMIC_TYPES, "(Annotation) anno_id is nullptr, but original member has "
                << (int)annotation.annotation_typeid()._d());
        }
        AnnotationDescriptor anno_desc;
        anno_desc.set_type(build_dynamic_type(get_type_name(anno_id), anno_id, get_type_object(anno_id)));
        const AppliedAnnotationParameterSeq& anno_params = annotation.param_seq();
        for (const AppliedAnnotationParameter& a_param : anno_params)
        {
            std::string param_key = get_key_from_hash(anno_desc.type(), a_param.paramname_hash());
            anno_desc.set_value(param_key, a_param.value().to_string());
        }
        parent_type_builder->apply_annotation_to_member(member_id, anno_desc);
    }
}

std::string TypeObjectFactory::get_key_from_hash(
        const DynamicType_ptr annotation_descriptor_type,
        const NameHash& hash) const
{
    std::map<MemberId, DynamicTypeMember*> members;
    annotation_descriptor_type->get_all_members(members);
    for (auto it : members)
    {
        std::string name = it.second->get_name();
        NameHash memberHash;
        MD5 message_hash(name);
        for(int i = 0; i < 4; ++i)
        {
            memberHash[i] = message_hash.digest[i];
        }
        if (memberHash == hash)
        {
            return name;
        }
    }
    return "";
}

TypeIdentifierWithSizeSeq TypeObjectFactory::typelookup_get_type_dependencies(
        const TypeIdentifierSeq& identifiers,
        const OctetSeq& in_continuation_point,
        OctetSeq& out_continuation_point,
        size_t max_size) const
{
    TypeIdentifierWithSizeSeq result;
    size_t continuation_point = to_size_t(in_continuation_point);
    size_t start_index = max_size * continuation_point;
    size_t skip = 0;

    // TODO Manage the overflow with additional "start_indexes" to cover
    // the complete 256 bits possibilities of OctetSeq.
    if (start_index < continuation_point)
    {
        // Overflow
        for(octet& o : out_continuation_point)
        {
            o = 0;
        }
        return result;
    }

    // For each given identifier
    for (const TypeIdentifier& identifier : identifiers)
    {
        const TypeIdentifier* local_id = get_stored_type_identifier(&identifier);

        // Check it is known
        if (local_id != nullptr)
        {
            // Create or retrieve its TypeInformation
            if (get_type_information(local_id) == nullptr)
            {
                TypeInformation aux;
                if (local_id->_d() > EK_MINIMAL)
                {
                    fill_complete_information(&aux, local_id);
                }
                else
                {
                    fill_minimal_information(&aux, local_id);
                }
            }
            const TypeInformation* local_info = get_type_information(local_id);
            if (local_info != nullptr)
            {
                // With the TypeInformation, retrieve directly their dependencies
                const TypeIdentifierWithSizeSeq& full_results =
                    (local_id->_d() > EK_MINIMAL)
                    ? local_info->complete().dependent_typeids()
                    : local_info->minimal().dependent_typeids();

                // Check the start_index
                if (skip + full_results.size() < start_index)
                {
                    skip += full_results.size();
                    continue;
                }
                else
                {
                    size_t local_start = start_index - skip;
                    size_t added = 0;
                    // Add the next identifiers.
                    for (size_t i = local_start; i < local_start + max_size && i < full_results.size(); ++i)
                    {
                        result.push_back(full_results[i]);
                        ++added;
                    }
                    skip = 0;
                    start_index = 0;
                    if (added == max_size)
                    {
                        // If max_size reached, increment out_continuation_point
                        ++out_continuation_point;
                        break;
                    }
                }
            }
        }
    }

    return result;
}

const TypeIdentifier* TypeObjectFactory::typelookup_get_type(
        const TypeIdentifier& identifier,
        TypeObject& object) const
{
    const TypeIdentifier* local_id = get_stored_type_identifier(&identifier);

    if (local_id != nullptr)
    {
        const TypeObject* local_obj = get_type_object(local_id);
        if (local_obj != nullptr)
        {
            object = *local_obj;
        }
    }

    return local_id;
}

bool TypeObjectFactory::typelookup_check_type_identifier(
        const TypeIdentifier& identifier) const
{
    return get_stored_type_identifier(&identifier) != nullptr;
}

const TypeObject* TypeObjectFactory::typelookup_get_type_object_from_information(
        const TypeInformation& information) const
{
    if (information.complete().typeid_with_size().type_id()._d() != 0)
    {
        const TypeIdentifier* local_id =
            get_stored_type_identifier(&information.complete().typeid_with_size().type_id());

        if (local_id != nullptr)
        {
            return get_type_object(local_id);
        }
    }
    return nullptr;
}

} // namespace types
} // namespace fastrtps
} // namespace eprosima
