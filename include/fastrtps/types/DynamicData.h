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

#ifndef TYPES_DYNAMIC_DATA_H
#define TYPES_DYNAMIC_DATA_H

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicTypePtr.h>

//#define DYNAMIC_TYPES_CHECKING

namespace eprosima {
namespace fastrtps {
namespace types {

class DynamicType;
class MemberDescriptor;

class DynamicData
{
protected:

    DynamicData();
    DynamicData(
            const DynamicData* pData);
    DynamicData(
            DynamicType_ptr pType);

    ~DynamicData();

    void add_value(
            TypeKind kind,
            MemberId id);

    void create_members(
            DynamicType_ptr pType);

    void create_members(
            const DynamicData* pData);

    void clean();

    void clean_members();

    void* clone_value(
            MemberId id,
            TypeKind kind) const;

    bool compare_values(
            TypeKind kind,
            void* left,
            void* right) const;

    ReturnCode_t insert_array_data(
            MemberId indexId);

    void serialize_empty_data(
            const DynamicType_ptr pType,
            eprosima::fastcdr::Cdr& cdr) const;

    void set_default_value(
            MemberId id);

    void get_value(
            std::string& sOutValue,
            MemberId id = MEMBER_ID_INVALID) const;

    void set_value(
            const std::string& sValue,
            MemberId id = MEMBER_ID_INVALID);

    void set_type_name(
            const std::string& name);

    MemberId get_union_id() const;

    ReturnCode_t set_union_id(
            MemberId id);

    void update_union_discriminator();

    void sort_member_ids(
            MemberId startId);

    void set_union_discriminator(
            DynamicData* pData);

    // Serializes and deserializes the Dynamic Data.
    bool deserialize(
            eprosima::fastcdr::Cdr& cdr);

    bool deserialize_discriminator(
            eprosima::fastcdr::Cdr& cdr);

    static size_t getCdrSerializedSize(
            const DynamicData* data,
            size_t current_alignment = 0);

    static size_t getEmptyCdrSerializedSize(
            const DynamicType* type,
            size_t current_alignment = 0);

    static size_t getKeyMaxCdrSerializedSize(
            const DynamicType_ptr type,
            size_t current_alignment = 0);

    static size_t getMaxCdrSerializedSize(
            const DynamicType_ptr type,
            size_t current_alignment = 0);

    void serialize(
            eprosima::fastcdr::Cdr& cdr) const;

    void serialize_discriminator(
            eprosima::fastcdr::Cdr& cdr) const;

    void serializeKey(
            eprosima::fastcdr::Cdr& cdr) const;

    DynamicType_ptr type_;
    std::map<MemberId, MemberDescriptor*> descriptors_;

#ifdef DYNAMIC_TYPES_CHECKING
    int32_t int32_value_;
    uint32_t uint32_value_;
    int16_t int16_value_;
    uint16_t uint16_value_;
    int64_t int64_value_;
    uint64_t uint64_value_;
    float float32_value_;
    double float64_value_;
    long double float128_value_;
    char char8_value_;
    wchar_t char16_value_;
    octet byte_value_;
    bool bool_value_;
    std::string string_value_;
    std::wstring wstring_value_;
    std::map<MemberId, DynamicData*> complex_values_;
#else
    std::map<MemberId, void*> values_;
#endif // ifdef DYNAMIC_TYPES_CHECKING
    std::vector<MemberId> loaned_values_;
    bool key_element_;
    DynamicData* default_array_value_;
    uint64_t union_label_;
    MemberId union_id_;
    DynamicData* union_discriminator_;
    uint64_t discriminator_value_;

    friend class DynamicDataFactory;
    friend class DynamicPubSubType;
    friend class DynamicDataHelper;

public:

    RTPS_DllAPI ReturnCode_t get_descriptor(
            MemberDescriptor& value,
            MemberId id);

    RTPS_DllAPI ReturnCode_t set_descriptor(
            MemberId id,
            const MemberDescriptor* value);

    RTPS_DllAPI ReturnCode_t clear_all_values();

    RTPS_DllAPI ReturnCode_t clear_nonkey_values();

    RTPS_DllAPI ReturnCode_t clear_value(
            MemberId id);

    RTPS_DllAPI bool equals(
            const DynamicData* other) const;

    RTPS_DllAPI TypeKind get_kind() const;

    RTPS_DllAPI uint32_t get_item_count() const;

    RTPS_DllAPI std::string get_name();

    RTPS_DllAPI MemberId get_member_id_by_name(
            const std::string& name) const;

    RTPS_DllAPI MemberId get_member_id_at_index(
            uint32_t index) const;

    RTPS_DllAPI DynamicData* loan_value(
            MemberId id);

    RTPS_DllAPI ReturnCode_t return_loaned_value(
            const DynamicData* value);

    RTPS_DllAPI MemberId get_array_index(
            const std::vector<uint32_t>& position);

    RTPS_DllAPI ReturnCode_t insert_sequence_data(
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_int32_value(
            int32_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_uint32_value(
            uint32_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_int16_value(
            int16_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_uint16_value(
            uint16_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_int64_value(
            int64_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_uint64_value(
            uint64_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_float32_value(
            float value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_float64_value(
            double value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_float128_value(
            long double value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_char8_value(
            char value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_char16_value(
            wchar_t value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_byte_value(
            octet value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_bool_value(
            bool value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_string_value(
            const std::string& value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_wstring_value(
            const std::wstring& value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_enum_value(
            const std::string& value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_complex_value(
            const DynamicData* value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_complex_value(
            DynamicData* value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t insert_complex_value(
            DynamicData_ptr value,
            MemberId& outId);

    RTPS_DllAPI ReturnCode_t remove_sequence_data(
            MemberId id);

    RTPS_DllAPI ReturnCode_t clear_data();

    RTPS_DllAPI ReturnCode_t clear_array_data(
            MemberId indexId);

    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData* key,
            MemberId& outKeyId,
            MemberId& outValueId);

    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData* key,
            DynamicData* value,
            MemberId& outKey,
            MemberId& outValue);

    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData* key,
            const DynamicData* value,
            MemberId& outKey,
            MemberId& outValue);

    RTPS_DllAPI ReturnCode_t insert_map_data(
            const DynamicData* key,
            DynamicData_ptr value,
            MemberId& outKey,
            MemberId& outValue);

    RTPS_DllAPI ReturnCode_t remove_map_data(
            MemberId keyId);

    RTPS_DllAPI ReturnCode_t get_int32_value(
            int32_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_int32_value(
            int32_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_uint32_value(
            uint32_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_uint32_value(
            uint32_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_int16_value(
            int16_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_int16_value(
            int16_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_uint16_value(
            uint16_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_uint16_value(
            uint16_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_int64_value(
            int64_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_int64_value(
            int64_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_uint64_value(
            uint64_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_uint64_value(
            uint64_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_float32_value(
            float& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_float32_value(
            float value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_float64_value(
            double& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_float64_value(
            double value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_float128_value(
            long double& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_float128_value(
            long double value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_char8_value(
            char& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_char8_value(
            char value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_char16_value(
            wchar_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_char16_value(
            wchar_t value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_byte_value(
            octet& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_byte_value(
            octet value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_int8_value(
            int8_t& value,
            MemberId id) const
    {
        octet aux;
        ReturnCode_t result = get_byte_value(aux, id);
        value = static_cast<int8_t>(aux);
        return result;
    }

    RTPS_DllAPI ReturnCode_t set_int8_value(
            int8_t value,
            MemberId id = MEMBER_ID_INVALID)
    {
        return set_byte_value(static_cast<octet>(value), id);
    }

    RTPS_DllAPI ReturnCode_t get_uint8_value(
            uint8_t& value,
            MemberId id) const
    {
        octet aux;
        ReturnCode_t result = get_byte_value(aux, id);
        value = static_cast<uint8_t>(aux);
        return result;
    }

    RTPS_DllAPI ReturnCode_t set_uint8_value(
            uint8_t value,
            MemberId id = MEMBER_ID_INVALID)
    {
        return set_byte_value(static_cast<octet>(value), id);
    }

    RTPS_DllAPI ReturnCode_t get_bool_value(
            bool& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_bool_value(
            bool value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t set_bool_value(
            bool value,
            const std::string& name)
    {
        MemberId id = get_member_id_by_name(name);
        if (id != MEMBER_ID_INVALID)
        {
            return set_bool_value(value, id);
        }
        return ReturnCode_t::RETCODE_BAD_PARAMETER;
    }

    RTPS_DllAPI ReturnCode_t get_string_value(
            std::string& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_string_value(
            const std::string& value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_wstring_value(
            std::wstring& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_wstring_value(
            const std::wstring& value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_enum_value(
            std::string& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_enum_value(
            const std::string& value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_enum_value(
            uint32_t& value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_enum_value(
            const uint32_t& value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_bitmask_value(
            uint64_t& value) const;

    RTPS_DllAPI uint64_t get_bitmask_value() const
    {
        uint64_t value;
        if (get_bitmask_value(value) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI ReturnCode_t set_bitmask_value(
            uint64_t value);

    RTPS_DllAPI ReturnCode_t get_complex_value(
            DynamicData** value,
            MemberId id) const;

    RTPS_DllAPI ReturnCode_t set_complex_value(
            DynamicData* value,
            MemberId id = MEMBER_ID_INVALID);

    RTPS_DllAPI ReturnCode_t get_union_label(
            uint64_t& value) const;

    // Basic types returns (copy)
    RTPS_DllAPI int32_t get_int32_value(
            MemberId id) const
    {
        int32_t value;
        if (get_int32_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint32_t get_uint32_value(
            MemberId id) const
    {
        uint32_t value;
        if (get_uint32_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI int16_t get_int16_value(
            MemberId id) const
    {
        int16_t value;
        if (get_int16_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint16_t get_uint16_value(
            MemberId id) const
    {
        uint16_t value;
        if (get_uint16_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI int64_t get_int64_value(
            MemberId id) const
    {
        int64_t value;
        if (get_int64_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint64_t get_uint64_value(
            MemberId id) const
    {
        uint64_t value;
        if (get_uint64_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI float get_float32_value(
            MemberId id) const
    {
        float value;
        if (get_float32_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI double get_float64_value(
            MemberId id) const
    {
        double value;
        if (get_float64_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI long double get_float128_value(
            MemberId id) const
    {
        long double value;
        if (get_float128_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI char get_char8_value(
            MemberId id) const
    {
        char value;
        if (get_char8_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI wchar_t get_char16_value(
            MemberId id) const
    {
        wchar_t value;
        if (get_char16_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI octet get_byte_value(
            MemberId id) const
    {
        octet value;
        if (get_byte_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI int8_t get_int8_value(
            MemberId id) const
    {
        int8_t value;
        if (get_int8_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint8_t get_uint8_value(
            MemberId id) const
    {
        uint8_t value;
        if (get_uint8_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI bool get_bool_value(
            MemberId id) const
    {
        bool value;
        if (get_bool_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI bool get_bool_value(
            const std::string& name) const
    {
        MemberId id = get_member_id_by_name(name);
        bool value;
        if (get_bool_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::string get_string_value(
            MemberId id) const
    {
        std::string value;
        if (get_string_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::wstring get_wstring_value(
            MemberId id) const
    {
        std::wstring value;
        if (get_wstring_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI std::string get_enum_value(
            MemberId id) const
    {
        std::string value;
        if (get_enum_value(value, id) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint64_t get_union_label() const
    {
        uint64_t value;
        if (get_union_label(value) != ReturnCode_t::RETCODE_OK)
        {
            throw ReturnCode_t::RETCODE_BAD_PARAMETER;
        }
        return value;
    }

    RTPS_DllAPI uint64_t get_discriminator_value() const
    {
        return discriminator_value_;
    }

    RTPS_DllAPI void get_discriminator_value(
            uint64_t& outValue) const
    {
        outValue = discriminator_value_;
    }

    RTPS_DllAPI void set_discriminator_value(
            uint64_t value)
    {
        discriminator_value_ = value;
    }

};


} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_DYNAMIC_DATA_H
