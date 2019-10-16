// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file TypeLookupTypes.hpp
 *
 */

#ifndef TYPELOOKUPTYPES_HPP
#define TYPELOOKUPTYPES_HPP

#include <fastrtps/types/TypeObject.h>
#include <fastdds/dds/builtin/common/RequestHeader.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <cstdint>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

const int32_t TypeLookup_getTypes_Hash = static_cast<int32_t>(0xd35282d1);
const int32_t TypeLookup_getDependencies_Hash = static_cast<int32_t>(0x31fbaa35);

struct TypeLookup_getTypes_In
{
    std::vector<fastrtps::types::TypeIdentifier> type_ids;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypes_In& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

struct TypeLookup_getTypes_Out
{
    std::vector<fastrtps::types::TypeIdentifierTypeObjectPair> types;

    std::vector<fastrtps::types::TypeIdentifierPair> complete_to_minimal;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypes_Out& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

class TypeLookup_getTypes_Result
{
public:

    RTPS_DllAPI TypeLookup_getTypes_Result();

    RTPS_DllAPI ~TypeLookup_getTypes_Result();

    RTPS_DllAPI TypeLookup_getTypes_Result(const TypeLookup_getTypes_Result &x);

    RTPS_DllAPI TypeLookup_getTypes_Result(TypeLookup_getTypes_Result &&x);

    RTPS_DllAPI TypeLookup_getTypes_Result& operator=(const TypeLookup_getTypes_Result &x);

    RTPS_DllAPI TypeLookup_getTypes_Result& operator=(TypeLookup_getTypes_Result &&x);

    RTPS_DllAPI void _d(int32_t __d);

    RTPS_DllAPI int32_t _d() const;

    RTPS_DllAPI int32_t& _d();

    RTPS_DllAPI void result(const TypeLookup_getTypes_Out &_result);

    RTPS_DllAPI void result(TypeLookup_getTypes_Out &&_result);

    RTPS_DllAPI const TypeLookup_getTypes_Out& result() const;

    RTPS_DllAPI TypeLookup_getTypes_Out& result();

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypes_Result& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }

private:
    int32_t m__d;
    TypeLookup_getTypes_Out m_result;
};

class TypeLookup_getTypeDependencies_In
{
public:
    std::vector<fastrtps::types::TypeIdentifier> type_ids;

    std::vector<uint8_t> continuation_point;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypeDependencies_In& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

class TypeLookup_getTypeDependencies_Out
{
public:
    std::vector<fastrtps::types::TypeIdentifierWithSize> dependent_typeids;

    std::vector<uint8_t> continuation_point;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypeDependencies_Out& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

class TypeLookup_getTypeDependencies_Result
{
public:

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result();

    RTPS_DllAPI ~TypeLookup_getTypeDependencies_Result();

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result(const TypeLookup_getTypeDependencies_Result &x);

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result(TypeLookup_getTypeDependencies_Result &&x);

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result& operator=(const TypeLookup_getTypeDependencies_Result &x);

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result& operator=(TypeLookup_getTypeDependencies_Result &&x);

    RTPS_DllAPI void _d(int32_t __d);

    RTPS_DllAPI int32_t _d() const;

    RTPS_DllAPI int32_t& _d();

    RTPS_DllAPI void result(const TypeLookup_getTypeDependencies_Out &_result);

    RTPS_DllAPI void result(TypeLookup_getTypeDependencies_Out &&_result);

    RTPS_DllAPI const TypeLookup_getTypeDependencies_Out& result() const;

    RTPS_DllAPI TypeLookup_getTypeDependencies_Out& result();

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_getTypeDependencies_Result& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }

private:
    int32_t m__d;

    TypeLookup_getTypeDependencies_Out m_result;
};
class TypeLookup_Call
{
public:

    RTPS_DllAPI TypeLookup_Call();

    RTPS_DllAPI ~TypeLookup_Call();

    RTPS_DllAPI TypeLookup_Call(const TypeLookup_Call &x);

    RTPS_DllAPI TypeLookup_Call(TypeLookup_Call &&x);

    RTPS_DllAPI TypeLookup_Call& operator=(const TypeLookup_Call &x);

    RTPS_DllAPI TypeLookup_Call& operator=(TypeLookup_Call &&x);

    RTPS_DllAPI void _d(int32_t __d);

    RTPS_DllAPI int32_t _d() const;

    RTPS_DllAPI int32_t& _d();

    RTPS_DllAPI void getTypes(const TypeLookup_getTypes_In &_getTypes);

    RTPS_DllAPI void getTypes(TypeLookup_getTypes_In &&_getTypes);

    RTPS_DllAPI const TypeLookup_getTypes_In& getTypes() const;

    RTPS_DllAPI TypeLookup_getTypes_In& getTypes();

    RTPS_DllAPI void getTypeDependencies(const TypeLookup_getTypeDependencies_In &_getTypeDependencies);

    RTPS_DllAPI void getTypeDependencies(TypeLookup_getTypeDependencies_In &&_getTypeDependencies);

    RTPS_DllAPI const TypeLookup_getTypeDependencies_In& getTypeDependencies() const;

    RTPS_DllAPI TypeLookup_getTypeDependencies_In& getTypeDependencies();

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_Call& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }

private:
    int32_t m__d;

    TypeLookup_getTypes_In m_getTypes;
    TypeLookup_getTypeDependencies_In m_getTypeDependencies;
};

class TypeLookup_Request
{
public:
    rpc::RequestHeader header;

    TypeLookup_Call data;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_Request& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

class TypeLookup_Return
{
public:

    RTPS_DllAPI TypeLookup_Return();

    RTPS_DllAPI ~TypeLookup_Return();

    RTPS_DllAPI TypeLookup_Return(const TypeLookup_Return &x);

    RTPS_DllAPI TypeLookup_Return(TypeLookup_Return &&x);

    RTPS_DllAPI TypeLookup_Return& operator=(const TypeLookup_Return &x);

    RTPS_DllAPI TypeLookup_Return& operator=(TypeLookup_Return &&x);

    RTPS_DllAPI void _d(int32_t __d);

    RTPS_DllAPI int32_t _d() const;

    RTPS_DllAPI int32_t& _d();

    RTPS_DllAPI void getType(const TypeLookup_getTypes_Result &_getType);

    RTPS_DllAPI void getType(TypeLookup_getTypes_Result &&_getType);

    RTPS_DllAPI const TypeLookup_getTypes_Result& getType() const;

    RTPS_DllAPI TypeLookup_getTypes_Result& getType();

    RTPS_DllAPI void getTypeDependencies(const TypeLookup_getTypeDependencies_Result &_getTypeDependencies);

    RTPS_DllAPI void getTypeDependencies(TypeLookup_getTypeDependencies_Result &&_getTypeDependencies);

    RTPS_DllAPI const TypeLookup_getTypeDependencies_Result& getTypeDependencies() const;

    RTPS_DllAPI TypeLookup_getTypeDependencies_Result& getTypeDependencies();

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_Return& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }

private:
    int32_t m__d;

    TypeLookup_getTypes_Result m_getType;
    TypeLookup_getTypeDependencies_Result m_getTypeDependencies;
};

class TypeLookup_Reply
{
public:
    rpc::RequestHeader header;

    TypeLookup_Return return_value;

    RTPS_DllAPI static size_t getCdrSerializedSize(const TypeLookup_Reply& data, size_t current_alignment = 0);

    RTPS_DllAPI void serialize(eprosima::fastcdr::Cdr &cdr) const;

    RTPS_DllAPI void deserialize(eprosima::fastcdr::Cdr &cdr);

    RTPS_DllAPI static bool isKeyDefined() { return false; }
};

// TypeSupports
class TypeLookup_RequestPubSubType : public TopicDataType
{
public:
    TypeLookup_RequestPubSubType();

    virtual ~TypeLookup_RequestPubSubType() override;

    bool serialize(
            void *data,
            fastrtps::rtps::SerializedPayload_t* payload) override;

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    /*
    bool getKey(
            void*,
            fastrtps::rtps::InstanceHandle_t*,
            bool) override
    {}
    */

    void* createData() override;

    void deleteData(void* data) override;
};

class TypeLookup_ReplyPubSubType : public TopicDataType
{
public:
    TypeLookup_ReplyPubSubType();

    virtual ~TypeLookup_ReplyPubSubType() override;

    bool serialize(
            void *data,
            fastrtps::rtps::SerializedPayload_t* payload) override;

    bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    /*
    bool getKey(
            void*,
            fastrtps::rtps::InstanceHandle_t*,
            bool) override
    {}
    */

    void* createData() override;

    void deleteData(void* data) override;
};

class TypeLookup_RequestTypeSupport : public TypeSupport
{
public:
    RTPS_DllAPI bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) override;

    RTPS_DllAPI bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    static size_t getCdrSerializedSize(
            const TypeLookup_Request& data,
            size_t current_alignment = 0);

    RTPS_DllAPI void* create_data() override;

    RTPS_DllAPI void delete_data(void* data) override;
};

class TypeLookup_ReplyTypeSupport : public TypeSupport
{
public:
    RTPS_DllAPI bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) override;

    RTPS_DllAPI bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    static size_t getCdrSerializedSize(
            const TypeLookup_Reply& data,
            size_t current_alignment = 0);

    RTPS_DllAPI void* create_data() override;

    RTPS_DllAPI void delete_data(void* data) override;
};


}
}
}
}

#endif // TYPELOOKUPTYPES_HPP
