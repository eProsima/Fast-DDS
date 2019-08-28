#include <fastrtps/serializer/serializer.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/MemberDescriptor.h>
#include <fastcdr/Cdr.h>
#include <fastrtps/types/TypeDescriptor.h>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/log/Log.h>

namespace eprosima{
namespace fastrtps{
namespace serializer{

using namespace eprosima::fastrtps::types ;

void Serializer::serialize(eprosima::fastcdr::Cdr& cdr) 
{
    switch (t_){
        case theType::DynamicData :
            serializeDynData(p_.dd(), cdr) ;
            break ;

        default:
            break ;
    }
}

void Serializer::deserialize(eprosima::fastcdr::Cdr& cdr) 
{
    switch (t_){
        case theType::DynamicData :
            deserializeDynData(p_.dd(), cdr) ;
            break ;

        default:
            break ;
    }
}

void Serializer::serializeDynData(DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const
{
    if ( dd->type() != nullptr && dd->type()->get_descriptor()->annotation_is_non_serialized())
    {
        return;
    }

    switch (dd->get_kind())
    {
    default:
        break;
    case TK_INT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_int32_t() ;
#else
        auto it = dd->values().begin();
        cdr << *((int32_t*)it->second);
#endif
        break;
    }
    case TK_UINT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_uint32_t();
#else
        auto it = dd->values().begin();
        cdr << *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_INT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_int16_t() ;
#else
        auto it = dd->values().begin();
        cdr << *((int16_t*)it->second);
#endif
        break;
    }
    case TK_UINT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_uint16_t() ;
#else
        auto it = dd->values().begin();
        cdr << *((uint16_t*)it->second);
#endif
        break;
    }
    case TK_INT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_int64_t();
#else
        auto it = dd->values().begin();
        cdr << *((int64_t*)it->second);
#endif
        break;
    }
    case TK_UINT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_uint64_t();
#else
        auto it = dd->values().begin();
        cdr << *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_FLOAT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_float32_t();
#else
        auto it = dd->values().begin();
        cdr << *((float*)it->second);
#endif
        break;
    }
    case TK_FLOAT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_float64_t() ;
#else
        auto it = dd->values().begin();
        cdr << *((double*)it->second);
#endif
        break;
    }
    case TK_FLOAT128:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_float128_t();
#else
        auto it = dd->values().begin();
        cdr << *((long double*)it->second);
#endif
        break;
    }
    case TK_CHAR8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_char8_t();
#else
        auto it = dd->values().begin();
        cdr << *((char*)it->second);
#endif
        break;
    }
    case TK_CHAR16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_char16_t();
#else
        auto it = dd->values().begin();
        cdr << *((wchar_t*)it->second);
#endif
        break;
    }
    case TK_BOOLEAN:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_bool_t();
#else
        auto it = dd->values().begin();
        cdr << *((bool*)it->second);
#endif
        break;
    }
    case TK_BYTE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_octet_t();
#else
        auto it = dd->values().begin();
        cdr << *((octet*)it->second);
#endif
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_string_t();
#else
        auto it = dd->values().begin();
        cdr << *((std::string*)it->second);
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_wstring_t();
#else
        auto it = dd->values().begin();
        cdr << *((std::wstring*)it->second);
#endif
        break;
    }
    case TK_ENUM:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << dd->get_uint32_t();
#else
        auto it = dd->values().begin();
        cdr << *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_BITMASK:
    {
        size_t type_size = dd->type()->get_size();
#ifdef DYNAMIC_TYPES_CHECKING
        switch (type_size)
        {
            case 1: cdr << (uint8_t)dd->get_uint64_t(); break;
            case 2: cdr << (uint16_t)dd->get_uint64_t(); break;
            case 3: cdr << (uint32_t)dd->get_uint64_t(); break;
            case 4: cdr << dd->get_uint64_t(); break;
            default: logError(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
        }
#else
        auto it = dd->values().begin();
        switch (type_size)
        {
            case 1: cdr << *((uint8_t*)it->second); break;
            case 2: cdr << *((uint16_t*)it->second); break;
            case 3: cdr << *((uint32_t*)it->second); break;
            case 4: cdr << *((uint64_t*)it->second); break;
            default: logError(DYN_TYPES, "Cannot serialize bitmask of size " << type_size);
        }
#endif
        break;
    }
    case TK_UNION:
    {
        serialize_discriminator( dd->get_union_discriminator(), cdr) ;
        //cdr << union_id_;
        if (dd->get_union_id() != MEMBER_ID_INVALID)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = dd->get_complex_t().at(dd->get_union_id());
#else
            auto it = (DynamicData*) dd->values().at(dd->get_union_id());
#endif
            serializeDynData(&(*it), cdr);
            //FRANAVA: it->serialize(cdr);
        }
        break;
    }
    case TK_SEQUENCE: // Sequence is like structure, but with size
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << static_cast<uint32_t>(dd->get_complex_t().size());
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(dd->get_complex_t().size()); ++idx)
        {
            auto it = dd->get_complex_t().at(idx);
            serializeDynData(&(*it), cdr);
            //FRANAVA: it->serialize(cdr);
        }
#else
        cdr << static_cast<uint32_t>(dd->values().size());
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(dd->values().size()); ++idx)
        {
            auto it = dd->values().at(idx);
            serializeDynData(((DynamicData*)it), cdr);
            //FRANAVA: ((DynamicData*)it)->serialize(cdr);
        }
#endif
        break;
    }
    case TK_STRUCTURE:
    case TK_BITSET:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(dd->get_complex_t().size()); ++idx)
        {
            auto d_it = dd->get_descriptors().find(idx);
            if (d_it != dd->get_descriptors().end())
            {
                const MemberDescriptor* member_desc = d_it->second;
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = dd->get_complex_t().at(idx);
                    Serializer(it).serialize(cdr);
																				
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << idx);
            }
        }
#else
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(dd->values().size()); ++idx)
        {
            auto d_it = dd->get_descriptors().find(idx);
            if (d_it != dd->get_descriptors().end())
            {
                const MemberDescriptor* member_desc = d_it->second;
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = dd->values().at(idx);
                    serializeDynData(((DynamicData*)it), cdr);
                    //FRANAVA: ((DynamicData*)it)->serialize(cdr);
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << idx);
            }
        }
#endif
        break;
    }
    case TK_ARRAY:
    {
        uint32_t arraySize = dd->type()->get_total_bounds();
        for (uint32_t idx = 0; idx < arraySize; ++idx)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = dd->get_complex_t().find(idx);
            if (it != dd->get_complex_t().end())
#else
            auto it = dd->values().find(idx);
            if (it != dd->values().end())
#endif
            {
                serializeDynData(((DynamicData*)it->second), cdr);
                //FRANAVA: ((DynamicData*)it->second)->serialize(cdr);
            }
            else
            {
//                serialize_empty_data(dd->type()->get_element_type(), cdr);
                serialize_empty_data(dd->type()->get_element_type().get(), cdr);
            }
        }
        break;
            }
    case TK_MAP:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr << static_cast<uint32_t>(dd->get_complex_t().size() / 2); // Number of pairs
        for (auto it = dd->get_complex_t().begin(); it != dd->get_complex_t().end(); ++it)
        {
            Serializer(it->second).serialize(cdr);
        }
#else
        cdr << static_cast<uint32_t>(dd->values().size() / 2);
        for (auto it = dd->values().begin(); it != dd->values().end(); ++it)
        {
            serializeDynData(((DynamicData*)it->second), cdr);
            //FRANAVA: ((DynamicData*)it->second)->serialize(cdr);
        }
#endif
        break;
    }
    case TK_ALIAS:
        break;
    }
}

void Serializer::serialize_discriminator(DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const
{
    switch (dd->get_kind())
    {
    case TK_INT32:
    {
        int32_t aux = static_cast<int32_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_UINT32:
    {
        uint32_t aux = static_cast<uint32_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_INT16:
    {
        int16_t aux = static_cast<int16_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_UINT16:
    {
        uint16_t aux = static_cast<uint16_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_INT64:
    {
        int64_t aux = static_cast<int64_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_UINT64:
    {
        uint64_t aux = static_cast<uint64_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_CHAR8:
    {
        char aux = static_cast<char>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_CHAR16:
    {
        wchar_t aux = static_cast<wchar_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_BOOLEAN:
    {
        bool aux = !!(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_BYTE:
    {
        octet aux = static_cast<octet>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_ENUM:
    {
        uint32_t aux = static_cast<uint32_t>(dd->get_discriminator_value());
        cdr << aux;
        break;
    }
    case TK_FLOAT32:
    case TK_FLOAT64:
    case TK_FLOAT128:
    case TK_STRING8:
    case TK_STRING16:
    case TK_BITMASK:
    case TK_UNION:
    case TK_SEQUENCE:
    case TK_STRUCTURE:
    case TK_BITSET:
    case TK_ARRAY:
    case TK_MAP:
    case TK_ALIAS:
    default:
        break;
    }
}


void Serializer::serialize_empty_data(DynamicType *dt, eprosima::fastcdr::Cdr& cdr) const
{
    if (dt->get_descriptor()->annotation_is_non_serialized())
    {
        return;
    }

    switch (dt->get_kind())
    {
        default:
            break;
        case TK_ALIAS:
        {
            serialize_empty_data(dt->get_base_type().get(), cdr);
            break;
        }
        case TK_INT32:
        {
            cdr << static_cast<int32_t>(0);
            break;
        }
        case TK_UINT32:
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_INT16:
        {
            cdr << static_cast<int16_t>(0);
            break;
        }
        case TK_UINT16:
        {
            cdr << static_cast<uint16_t>(0);
            break;
        }
        case TK_INT64:
        {
            cdr << static_cast<int64_t>(0);
            break;
        }
        case TK_UINT64:
        {
            cdr << static_cast<uint64_t>(0);
            break;
        }
        case TK_FLOAT32:
        {
            cdr << static_cast<float>(0.0f);
            break;
        }
        case TK_FLOAT64:
        {
            cdr << static_cast<double>(0.0);
            break;
        }
        case TK_FLOAT128:
        {
            cdr << static_cast<long double>(0.0);
            break;
        }
        case TK_CHAR8:
        {
            cdr << static_cast<char>(0);
            break;
        }
        case TK_CHAR16:
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_BOOLEAN:
        {
            cdr << static_cast<uint8_t>(0);
            break;
        }
        case TK_BYTE:
        {
            cdr << static_cast<uint8_t>(0);
            break;
        }
        case TK_STRING8:
        {
            cdr << std::string();
            break;
        }
        case TK_STRING16:
        {
            cdr << std::wstring();
            break;
        }
        case TK_ENUM:
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_BITMASK:
        {
            size_t type_size = dt->get_size();
            switch (type_size)
            {
                case 1: cdr << static_cast<uint8_t>(0); break;
                case 2: cdr << static_cast<uint16_t>(0); break;
                case 3: cdr << static_cast<uint32_t>(0); break;
                case 4: cdr << static_cast<uint64_t>(0); break;
                default: logError(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
            }
            break;
        }
        case TK_UNION:
        {
            cdr << static_cast<uint32_t>(MEMBER_ID_INVALID);
            break;
        }
        case TK_SEQUENCE: // Sequence is like structure, but with size
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
        case TK_STRUCTURE:
        case TK_BITSET:
        {
            for (uint32_t idx = 0; idx < dt->get_member_by_id().size(); ++idx)
            {
                auto it = dt->get_member_by_id().at(idx); //DynamicTypeMember *
                if (!it->get_descriptor()->annotation_is_non_serialized())
                {
                    serialize_empty_data(it->get_descriptor()->get_type().get(), cdr);
                }
            }
            break;
        }
        case TK_ARRAY:
        {
            uint32_t arraySize = dt->get_total_bounds();
            //cdr << arraySize;
            for (uint32_t i = 0; i < arraySize; ++i)
            {
                serialize_empty_data(dt->get_element_type().get(), cdr);
            }
            break;
        }
        case TK_MAP:
        {
            cdr << static_cast<uint32_t>(0);
            break;
        }
    }
}


bool Serializer::deserializeDynData(DynamicData *dd, eprosima::fastcdr::Cdr& cdr)const
{
    if (dd->type() != nullptr && dd->type()->get_descriptor()->annotation_is_non_serialized())
    {
        return true;
    }

    switch (dd->get_kind())
    {
    default:
        break;
    case TK_INT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_int32_t();

#else
        auto it = dd->values().begin();
        cdr >> *((int32_t*)it->second);
#endif
        break;
    }
    case TK_UINT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_uint32_t();

#else
        auto it = dd->values().begin();
        cdr >> *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_INT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_int16_t();

#else
        auto it = dd->values().begin();
        cdr >> *((int16_t*)it->second);
#endif
        break;
    }
    case TK_UINT16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_uint16_t();

#else
        auto it = dd->values().begin();
        cdr >> *((uint16_t*)it->second);
#endif
        break;
    }
    case TK_INT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_int64_t();

#else
        auto it = dd->values().begin();
        cdr >> *((int64_t*)it->second);
#endif
        break;
    }
    case TK_UINT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_uint64_t();

#else
        auto it = dd->values().begin();
        cdr >> *((uint64_t*)it->second);
#endif
        break;
    }
    case TK_FLOAT32:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_float32_t();
#else
        auto it = dd->values().begin();
        cdr >> *((float*)it->second);
#endif
        break;
    }
    case TK_FLOAT64:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_float64_t();

#else
        auto it = dd->values().begin();
        cdr >> *((double*)it->second);
#endif
        break;
    }
    case TK_FLOAT128:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_float128_t();

#else
        auto it = dd->values().begin();
        cdr >> *((long double*)it->second);
#endif
        break;
    }
    case TK_CHAR8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_char8_t();

#else
        auto it = dd->values().begin();
        cdr >> *((char*)it->second);
#endif
        break;
    }
    case TK_CHAR16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_char16_t();

#else
        auto it = dd->values().begin();
        cdr >> *((wchar_t*)it->second);
#endif
        break;
    }
    case TK_BOOLEAN:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_bool_t();

#else
        auto it = dd->values().begin();
        cdr >> *((bool*)it->second);
#endif
        break;
    }
    case TK_BYTE:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_octet_t();

#else
        auto it = dd->values().begin();
        cdr >> *((octet*)it->second);
#endif
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_string_t();

#else
        auto it = dd->values().begin();
        cdr >> *((std::string*)it->second);
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_wstring_t();

#else
        auto it = dd->values().begin();
        cdr >> *((std::wstring*)it->second);
#endif
        break;
    }
    case TK_ENUM:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        cdr >> dd->assign_to_uint32_t() ;

#else
        auto it = dd->values().begin();
        cdr >> *((uint32_t*)it->second);
#endif
        break;
    }
    case TK_BITMASK:
    {
        size_t type_size = dd->type()->get_size();
#ifdef DYNAMIC_TYPES_CHECKING
        switch (type_size)
        {
            case 1:
            {
                uint8_t temp;
                cdr >> temp;
                dd->assign_to_uint64_t() = temp;
                break;
            }
            case 2:
            {
                uint16_t temp;
                cdr >> temp;
                dd->assign_to_uint64_t() = temp;
                break;
            }
            case 3:
            {
                uint32_t temp;
                cdr >> temp;
                dd->assign_to_uint64_t() = temp;
                break;
            }
            case 4: cdr >> dd->assign_to_uint64_t(); break;
            default: logError(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
        }
#else
        auto it = dd->values().begin();
        switch (type_size)
        {
            case 1: cdr >> *((uint8_t*)it->second); break;
            case 2: cdr >> *((uint16_t*)it->second); break;
            case 3: cdr >> *((uint32_t*)it->second); break;
            case 4: cdr >> *((uint64_t*)it->second); break;
            default: logError(DYN_TYPES, "Cannot deserialize bitmask of size " << type_size);
        }
#endif
        break;
    }
    case TK_UNION:
    {
        dd->get_union_discriminator()->deserialize_discriminator(cdr);
        dd->update_union_discriminator();
        dd->set_union_id(dd->get_union_id());
        if (dd->get_union_id() != MEMBER_ID_INVALID)
        {

#ifdef DYNAMIC_TYPES_CHECKING
            auto it = dd->get_complex_t().find(dd->get_union_id());
            if (it != dd->get_complex_t().end())
            {
                Serializer(it->second).deserialize(cdr);
            }
#else
            auto it = dd->values().find(dd->get_union_id());
            if (it != dd->values().end())
            {
                Serializer((DynamicData*)it->second).deserialize(cdr) ;
            }
#endif
        }
        break;
    }
    case TK_STRUCTURE:
    case TK_BITSET:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        //uint32_t size(static_cast<uint32_t>(dd->get_complex_t().size())), memberId(MEMBER_ID_INVALID);
        for (uint32_t i = 0; i < dd->get_complex_t().size(); ++i)
        {
            //cdr >> memberId;
            MemberDescriptor* member_desc = dd->get_descriptors()[i];
            if (member_desc != nullptr)
            {
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = dd->get_complex_t().find(i);
                    if (it != dd->get_complex_t().end())
                    {
                        Serializer(it->second).deserialize(cdr);
                    }
                    else
                    {
                        DynamicData* pData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_element_type());
                        Serializer(pData).deserialize(cdr);
                        dd->get_complex_t().insert(std::make_pair(i, pData));
                    }
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << i);
            }
        }
#else
        //uint32_t size(static_cast<uint32_t>(values_.size())), memberId(MEMBER_ID_INVALID);
        for (uint32_t i = 0; i < dd->values().size(); ++i)
        {
            //cdr >> memberId;
            MemberDescriptor* member_desc = dd->get_descriptors()[i];
            if (member_desc != nullptr)
            {
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = dd->values().find(i);
                    if (it != dd->values().end())
                    {
                        Serializer((DynamicData*)it->second).deserialize(cdr);
                    }
                    else
                    {
                        DynamicData* pData = DynamicDataFactory::get_instance()->create_data( dd->type()->get_element_type() );
                        Serializer(pData).deserialize(cdr);
                        dd->values().insert(std::make_pair(i, pData));
                    }
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << i);
            }
        }
#endif
    }
    break;
    case TK_ARRAY:
    {
        uint32_t size(dd->type()->get_total_bounds());
        if (size > 0)
        {
            DynamicData* inputData(nullptr);
            for (uint32_t i = 0; i < size; ++i)
            {
#ifdef DYNAMIC_TYPES_CHECKING
                auto it = dd->get_complex_t().find(i);
                if (it != dd->get_complex_t().end())
                {
                    Serializer(it->second).deserialize(cdr);
                }
                else
                {
                    if (inputData == nullptr)
                    {
                        inputData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_element_type());
                    }

                    Serializer(inputData).deserialize(cdr);
                    if (!inputData->equals(dd->get_default_array_value()))
                    {
                        dd->get_complex_t().insert(std::make_pair(i, inputData));
                        inputData = nullptr;
                    }
                }
#else
                auto it = dd->values().find(i);
                if (it != dd->values().end())
                {
                    Serializer((DynamicData*)it->second).deserialize(cdr);
                }
                else
                {
                    if (inputData == nullptr)
                    {
                        inputData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_element_type());
                    }
                    Serializer(inputData).deserialize(cdr);
                    if (!inputData->equals(dd->get_default_array_value()))
                    {
                        dd->values().insert(std::make_pair(i, inputData));
                        inputData = nullptr;
                    }
                }
#endif
            }
            if (inputData != nullptr)
            {
                DynamicDataFactory::get_instance()->delete_data(inputData);
            }
        }
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        uint32_t size(0);
        bool bKeyElement(false);
        cdr >> size;

        if (dd->get_kind() == TK_MAP)
        {
            size *= 2; // We serialize the number of pairs.
        }
        for (uint32_t i = 0; i < size; ++i)
        {
            //cdr >> memberId;
            if (dd->get_kind() == TK_MAP)
            {
                bKeyElement = !bKeyElement;
            }

#ifdef DYNAMIC_TYPES_CHECKING
            auto it = dd->get_complex_t().find(i);
            if (it != dd->get_complex_t().end())
            {
                Serializer(it->second).deserialize(cdr);
                it->second->set_key_element(bKeyElement) ;
            }
            else
            {
                DynamicData* pData = nullptr;
                if (bKeyElement)
                {
                    pData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_key_element_type());
                }
                else
                {
                    pData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_element_type());
                }
                Serializer(pData).deserialize(cdr);
                pData->set_key_element(bKeyElement);
                dd->get_complex_t().insert(std::make_pair(i, pData));
            }
#else
            auto it = dd->values().find(i);
            if (it != dd->values().end())
            {
                Serializer((DynamicData*)it->second).deserialize(cdr);
                ((DynamicData*)it->second)->set_key_element(bKeyElement) ;
            }
            else
            {
                DynamicData* pData = nullptr;
                if (bKeyElement)
                {
                    pData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_key_element_type());
                }
                else
                {
                    pData = DynamicDataFactory::get_instance()->create_data(dd->type()->get_element_type());
                }
                Serializer(pData).deserialize(cdr);
                pData->set_key_element(bKeyElement);
                dd->values().insert(std::make_pair(i, pData));
            }
#endif
        }
        break;
    }

    case TK_ALIAS:
        break;
    }
    return true;
}


bool Serializer::deserialize_discriminator(eprosima::fastcdr::Cdr& cdr)
{
    switch (p_.dd()->get_kind())
    {
    case TK_INT32:
    {
        int32_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<int32_t>(aux));
        break;
    }
    case TK_UINT32:
    {
        uint32_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<uint32_t>(aux));
        break;
    }
    case TK_INT16:
    {
        int16_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<int16_t>(aux));
        break;
    }
    case TK_UINT16:
    {
        uint16_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<uint16_t>(aux));
        break;
    }
    case TK_INT64:
    {
        int64_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<int64_t>(aux));
        break;
    }
    case TK_UINT64:
    {
        uint64_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<uint64_t>(aux));
        break;
    }
    case TK_CHAR8:
    {
        char aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<char>(aux));
        break;
    }
    case TK_CHAR16:
    {
        wchar_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<wchar_t>(aux));
        break;
    }
    case TK_BOOLEAN:
    {
        bool aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<bool>(aux));
        break;
    }
    case TK_BYTE:
    {
        octet aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<octet>(aux));
        break;
    }
    case TK_ENUM:
    {
        uint32_t aux;
        cdr >> aux;
        p_.dd()->set_discriminator_value(static_cast<uint32_t>(aux));
        break;
    }
    case TK_FLOAT32:
    case TK_FLOAT64:
    case TK_FLOAT128:
    case TK_STRING8:
    case TK_STRING16:
    case TK_BITMASK:
    case TK_UNION:
    case TK_STRUCTURE:
    case TK_BITSET:
    case TK_ARRAY:
    case TK_SEQUENCE:
    case TK_MAP:
    case TK_ALIAS:
    default:
        break;
    }
    return true;
}

void Serializer::serializeKeyDynData(eprosima::fastcdr::Cdr& cdr) const
{
    // Structures check the the size of the key for their children
    if (p_.dd()->type()->get_kind() == TK_STRUCTURE || p_.dd()->type()->get_kind() == TK_BITSET)
    {
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = p_.dd()->get_complex_t().begin(); it != p_.dd()->get_complex_t().end(); ++it)
        {
            Serializer(it->second).serializeKeyDynData(cdr);
        }
#else
        for (auto it = p_.dd()->values().begin(); it != p_.dd()->values().end(); ++it)
        {
            Serializer((DynamicData*)it->second).serializeKeyDynData(cdr);
        }
#endif
    }
    else if (p_.dd()->type()->is_key_defined())
    {
				    //New Serialize instance to avoid Const/this issues
        Serializer(p_.dd()).serialize(cdr);
    }
}

size_t Serializer::getCdrSerializedSize(
        const DynamicData* data,
        size_t current_alignment /*= 0*/)
{
    if (data->type() != nullptr && data->type()->get_descriptor()->annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (data->get_kind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_BITMASK:
    {
        size_t type_size = data->type()->get_size();
        current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        // string content (length + characters + 1)
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            data->get_string_t().length() + 1;
#else
        auto it = data->values().begin();
        // string content (length + characters + 1)
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            ((std::string*)it->second)->length() + 1;
#endif
        break;
    }
    case TK_STRING16:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        // string content (length + (characters * 4) )
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            ((data->get_wstring_t().length()) * 4);
#else
        auto it = data->values().begin();
        // string content (length + (characters * 4) )
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) +
            (((std::wstring*)it->second)->length() * 4);
#endif
        break;
    }
    case TK_UNION:
    {
        // Union discriminator
        current_alignment += getCdrSerializedSize(data->get_union_discriminator(), current_alignment);

        if (data->get_union_id() != MEMBER_ID_INVALID)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = data->get_complex_t().at(data->get_union_id());
#else
            auto it = (DynamicData*)data->values().at(data->get_union_id());
#endif
            current_alignment += getCdrSerializedSize(it, current_alignment);
        }
        break;
    }
    case TK_STRUCTURE:
    case TK_BITSET:
    {
#ifdef DYNAMIC_TYPES_CHECKING
        //for (auto it = data->complex_values_.begin(); it != data->complex_values_.end(); ++it)
        //{
        //    current_alignment += getCdrSerializedSize(it->second, current_alignment);
        //}
        for (uint32_t i = 0; i < data->get_complex_t().size(); ++i)
        {
            //cdr >> memberId;
            auto d_it = data->get_descriptors().find(i);
            if (d_it != data->get_descriptors().end())
            {
                const MemberDescriptor* member_desc = d_it->second;
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = data->get_complex_t().find(i);
                    if (it != data->get_complex_t().end())
                    {
                        current_alignment += getCdrSerializedSize(it->second, current_alignment);
                    }
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << i);
            }
        }

#else
        //for (auto it = data->values_.begin(); it != data->values_.end(); ++it)
        //{
        //    current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
        //}
        for (uint32_t i = 0; i < data->values().size(); ++i)
        {
            //cdr >> memberId;
            auto d_it = data->get_descriptors().find(i);
            if (d_it != data->get_descriptors().end())
            {
                const MemberDescriptor* member_desc = d_it->second;
                if (!member_desc->annotation_is_non_serialized())
                {
                    auto it = data->values().find(i);
                    if (it != data->values().end())
                    {
                        current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
                    }
                }
            }
            else
            {
                logError(DYN_TYPES, "Missing MemberDescriptor " << i);
            }
        }
#endif
        break;
    }
    case TK_ARRAY:
    {
        uint32_t arraySize = data->type()->get_total_bounds();
        size_t emptyElementSize = getEmptyCdrSerializedSize(data->type()->get_element_type().get(), current_alignment);
        for (uint32_t idx = 0; idx < arraySize; ++idx)
        {
#ifdef DYNAMIC_TYPES_CHECKING
            auto it = data->get_complex_t().find(idx);
            if (it != data->get_complex_t().end())
#else
            auto it = data->values().find(idx);
            if (it != data->values().end())
#endif
            {
                // Element Size
                current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
            }
            else
            {
                current_alignment += emptyElementSize;
            }
        }
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
#ifdef DYNAMIC_TYPES_CHECKING
        for (auto it = data->get_complex_t().begin(); it != data->get_complex_t().end(); ++it)
        {
            // Element Size
            current_alignment += getCdrSerializedSize(it->second, current_alignment);
        }
#else
        for (auto it = data->values().begin(); it != data->values().end(); ++it)
        {
            // Element Size
            current_alignment += getCdrSerializedSize((DynamicData*)it->second, current_alignment);
        }
#endif
        break;
    }
    case TK_ALIAS:
        break;
    }

    return current_alignment - initial_alignment;
}


size_t Serializer::getEmptyCdrSerializedSize(
        const DynamicType* type,
        size_t current_alignment /*= 0*/)
{
    if (type->get_descriptor()->annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (type->get_kind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_BITMASK:
    {
        size_t type_size = type->get_size();
        current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 16);
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
        // string length + 1
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + 1;
        break;
    }
    case TK_STRING16:
    {
        // string length
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_UNION:
    {
        // union discriminator
        current_alignment += getEmptyCdrSerializedSize(type->get_discriminator_type().get(), current_alignment);
        break;
    }
    case TK_STRUCTURE:
    case TK_BITSET:
    {
        for (auto it = type->get_member_by_id().begin(); it != type->get_member_by_id().end(); ++it)
        {
            if (!it->second->get_descriptor()->annotation_is_non_serialized())
            {
                current_alignment += getEmptyCdrSerializedSize(it->second->get_descriptor()->get_type().get(), current_alignment);
            }
        }
        break;
    }
    case TK_ARRAY:
    {
        // Elements count
        //current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Element size with the maximum size
        current_alignment += type->get_total_bounds() * (getEmptyCdrSerializedSize(type->get_descriptor()->get_element_type().get()));
        break;
    }
    case TK_SEQUENCE:
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }

    case TK_ALIAS:
        current_alignment += getEmptyCdrSerializedSize(type->get_base_type().get());
        break;
    }

    return current_alignment - initial_alignment;
}

size_t Serializer::getKeyMaxCdrSerializedSize(
    const eprosima::fastrtps::types::DynamicType_ptr type,
    size_t current_alignment /*= 0*/)
{
    size_t initial_alignment = current_alignment;

    // Structures check the the size of the key for their children
    if (type->get_kind() == TK_STRUCTURE || type->get_kind() == TK_BITSET)
    {
        for (auto it = type->get_member_by_id().begin(); it != type->get_member_by_id().end(); ++it)
        {
            if (it->second->key_annotation())
            {
                current_alignment += getKeyMaxCdrSerializedSize(it->second->get_descriptor()->get_type(), current_alignment);
            }
        }
    }
    else if (type->is_key_defined())
    {
        return getMaxCdrSerializedSize(type, current_alignment);
    }
    return current_alignment - initial_alignment;
}

size_t Serializer::getMaxCdrSerializedSize(
    const eprosima::fastrtps::types::DynamicType_ptr type,
    size_t current_alignment /*= 0*/)
{
    if (type->get_descriptor()->annotation_is_non_serialized())
    {
        return 0;
    }

    size_t initial_alignment = current_alignment;

    switch (type->get_kind())
    {
    default:
        break;
    case TK_INT32:
    case TK_UINT32:
    case TK_FLOAT32:
    case TK_ENUM:
    case TK_CHAR16: // WCHARS NEED 32 Bits on Linux & MacOS
    {
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
        break;
    }
    case TK_INT16:
    case TK_UINT16:
    {
        current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
        break;
    }
    case TK_INT64:
    case TK_UINT64:
    case TK_FLOAT64:
    {
        current_alignment += 8 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_BITMASK:
    {
        size_t type_size = type->get_size();
        current_alignment += type_size + eprosima::fastcdr::Cdr::alignment(current_alignment, type_size);
        break;
    }
    case TK_FLOAT128:
    {
        current_alignment += 16 + eprosima::fastcdr::Cdr::alignment(current_alignment, 8);
        break;
    }
    case TK_CHAR8:
    case TK_BOOLEAN:
    case TK_BYTE:
    {
        current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
        break;
    }
    case TK_STRING8:
    {
        // string length + string content + 1
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + type->get_bounds() + 1;
        break;
    }
    case TK_STRING16:
    {
        // string length + ( string content * 4 )
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + (type->get_bounds() * 4);

        break;
    }
    case TK_UNION:
    {
        // union id
        current_alignment += getMaxCdrSerializedSize(type->get_discriminator_type(), current_alignment);

        // Check the size of all members and take the size of the biggest one.
        size_t temp_size(0);
        size_t max_element_size(0);
        for (auto it = type->get_member_by_id().begin(); it != type->get_member_by_id().end(); ++it)
        {
            temp_size = getMaxCdrSerializedSize(it->second->get_descriptor()->get_type(), current_alignment);
            if (temp_size > max_element_size)
            {
                max_element_size = temp_size;
            }
        }
        current_alignment += max_element_size;
        break;
    }
    case TK_STRUCTURE:
    case TK_BITSET:
    {
        for (auto it = type->get_member_by_id().begin(); it != type->get_member_by_id().end(); ++it)
        {
            if (!it->second->get_descriptor()->annotation_is_non_serialized())
            {
                current_alignment += getMaxCdrSerializedSize(it->second->get_descriptor()->get_type(), current_alignment);
            }
        }
        break;
    }
    case TK_ARRAY:
    {
        // Element size with the maximum size
        current_alignment += type->get_total_bounds() * (getMaxCdrSerializedSize(type->get_descriptor()->get_element_type()));
        break;
    }
    case TK_SEQUENCE:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Element size with the maximum size
        current_alignment += type->get_total_bounds() * (getMaxCdrSerializedSize(type->get_descriptor()->get_element_type()));
        break;
    }
    case TK_MAP:
    {
        // Elements count
        current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

        // Key Elements size with the maximum size
        current_alignment += type->get_total_bounds() * (getMaxCdrSerializedSize(type->get_descriptor()->get_key_element_type()));

        // Value Elements size with the maximum size
        current_alignment += type->get_total_bounds() * (getMaxCdrSerializedSize(type->get_descriptor()->get_element_type()));
        break;
    }

    case TK_ALIAS:
    {
        current_alignment += getMaxCdrSerializedSize(type->get_base_type());
        break;
    }
    }

    return current_alignment - initial_alignment;
}

void Serializer::serializeMemberFlag(MemberFlag *mf, eprosima::fastcdr::Cdr &cdr) const 
{
    //cdr << m_MemberFlag;
    uint16_t bits = static_cast<uint16_t>(mf->getM_memFlag().to_ulong());
    cdr << bits;
}

void Serializer::deserializeMemberFlag(MemberFlag *mf, eprosima::fastcdr::Cdr &cdr)
{
    //cdr >> (uint16_t)m_MemberFlag;
    uint16_t bits;
    cdr >> bits;
    mf->setM_memFlag(bits);
}

size_t Serializer::getCdrSerializedSize(const MemberFlag &mf, size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}

void Serializer::serializeTypeFlag(TypeFlag *tf, eprosima::fastcdr::Cdr &cdr) const
{
    //cdr << m_TypeFlag;
    uint16_t bits = static_cast<uint16_t>(tf->getM_typeFlag().to_ulong());
    cdr << bits;
}

void Serializer::deserializeTypeFlag(TypeFlag *tf, eprosima::fastcdr::Cdr &cdr)
{
    //cdr >> (uint16_t)m_TypeFlag;
    uint16_t bits;
    cdr >> bits;
    tf->setM_typeFlag(bits);
}

size_t Serializer::getCdrSerializedSize(const TypeFlag &tf, size_t current_alignment)
{
    return 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
}


size_t Serializer::getCdrSerializedSize(const TypeObjectHashId& data, size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch (data.get_m_d())
    {
        case EK_COMPLETE:
        case EK_MINIMAL:
            current_alignment += ((14) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1); break;
        default:
            break;
    }

    return current_alignment - initial_alignment;
}

void Serializer::serializeTypeObjectHashId(TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &scdr) const
{
    scdr << tohi->get_m_d() ;
    switch (tohi->get_m_d())
    {
        case EK_COMPLETE:
        case EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                scdr << tohi->get_m_hash()[i];
            }
            break;
        default:
            break;
    }
}


void Serializer::deserializeTypeObjectHashId(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr)
{
    uint8_t m_d;
    cdr >> m_d;
				tohi->set_m_d(m_d);

    switch (tohi->get_m_d())
    {
        case EK_COMPLETE:
        case EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                cdr >> tohi->get_m_hash()[i];
            }
            break;
        default:
            break;
    }
}


} //namespace serializer
} //namespace types
} //namespace eprosima
