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

#include <fastdds/dds/builtin/typelookup/common/TypeLookupTypes.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/BadParamException.h>

using namespace eprosima;
using namespace eprosima::fastcdr;
using namespace eprosima::fastcdr::exception;
using namespace eprosima::fastdds::dds::builtin;

using eprosima::fastrtps::types::TypeIdentifier;
using eprosima::fastrtps::types::TypeIdentifierWithSize;
using eprosima::fastrtps::types::TypeIdentifierPair;
using eprosima::fastrtps::types::TypeIdentifierTypeObjectPair;

TypeLookup_getTypes_Result::TypeLookup_getTypes_Result()
{
    m__d = 0;
}

TypeLookup_getTypes_Result::~TypeLookup_getTypes_Result()
{
}

TypeLookup_getTypes_Result::TypeLookup_getTypes_Result(const TypeLookup_getTypes_Result &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0:
        m_result = x.m_result;
        break;
    default:
        break;
    }
}

TypeLookup_getTypes_Result::TypeLookup_getTypes_Result(TypeLookup_getTypes_Result &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0:
        m_result = std::move(x.m_result);
        break;
    default:
        break;
    }
}

TypeLookup_getTypes_Result& TypeLookup_getTypes_Result::operator=(const TypeLookup_getTypes_Result &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0:
        m_result = x.m_result;
        break;
    default:
        break;
    }

    return *this;
}

TypeLookup_getTypes_Result& TypeLookup_getTypes_Result::operator=(TypeLookup_getTypes_Result &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0:
        m_result = std::move(x.m_result);
        break;
    default:
        break;
    }

    return *this;
}

void TypeLookup_getTypes_Result::_d(int32_t __d)
{
    m__d = __d;
}

int32_t TypeLookup_getTypes_Result::_d() const
{
    return m__d;
}

int32_t& TypeLookup_getTypes_Result::_d()
{
    return m__d;
}

void TypeLookup_getTypes_Result::result(const TypeLookup_getTypes_Out &_result)
{
    m_result = _result;
    m__d = 0;
}

void TypeLookup_getTypes_Result::result(TypeLookup_getTypes_Out &&_result)
{
    m_result = std::move(_result);
    m__d = 0;
}

const TypeLookup_getTypes_Out& TypeLookup_getTypes_Result::result() const
{
    bool b = false;

    switch(m__d)
    {
    case 0:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_result;
}

TypeLookup_getTypes_Out& TypeLookup_getTypes_Result::result()
{
    bool b = false;

    switch(m__d)
    {
    case 0:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_result;
}

size_t TypeLookup_getTypes_Result::getCdrSerializedSize(const TypeLookup_getTypes_Result& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch(data.m__d)
    {
    case 0:
        current_alignment += TypeLookup_getTypes_Out::getCdrSerializedSize(data.result(), current_alignment);
        break;
    default:
        break;
    }

    return current_alignment - initial_alignment;
}

void TypeLookup_getTypes_Result::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m__d;

    switch(m__d)
    {
    case 0:
        scdr << m_result;
        break;
    default:
        break;
    }
}

void TypeLookup_getTypes_Result::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m__d;

    switch(m__d)
    {
    case 0:
        dcdr >> m_result;
        break;
    default:
        break;
    }
}

size_t TypeLookup_getTypes_In::getCdrSerializedSize(const TypeLookup_getTypes_In& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.type_ids.size(); ++a)
    {
        current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_ids.at(a), current_alignment);}

    return current_alignment - initial_alignment;
}

void TypeLookup_getTypes_In::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << type_ids;}

void TypeLookup_getTypes_In::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> type_ids;}

size_t TypeLookup_getTypes_Out::getCdrSerializedSize(const TypeLookup_getTypes_Out& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.types.size(); ++a)
    {
        current_alignment += TypeIdentifierTypeObjectPair::getCdrSerializedSize(data.types.at(a), current_alignment);}

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.complete_to_minimal.size(); ++a)
    {
        current_alignment += TypeIdentifierPair::getCdrSerializedSize(data.complete_to_minimal.at(a), current_alignment);}


    return current_alignment - initial_alignment;
}

void TypeLookup_getTypes_Out::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << types;
    scdr << complete_to_minimal;
}

void TypeLookup_getTypes_Out::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> types;
    dcdr >> complete_to_minimal;
}

size_t TypeLookup_getTypeDependencies_In::getCdrSerializedSize(const TypeLookup_getTypeDependencies_In& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.type_ids.size(); ++a)
    {
        current_alignment += TypeIdentifier::getCdrSerializedSize(data.type_ids.at(a), current_alignment);}

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (data.continuation_point.size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);




    return current_alignment - initial_alignment;
}

void TypeLookup_getTypeDependencies_In::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << type_ids;
    scdr << continuation_point;
}

void TypeLookup_getTypeDependencies_In::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> type_ids;
    dcdr >> continuation_point;
}

size_t TypeLookup_getTypeDependencies_Out::getCdrSerializedSize(const TypeLookup_getTypeDependencies_Out& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    for(size_t a = 0; a < data.dependent_typeids.size(); ++a)
    {
        current_alignment += TypeIdentifierWithSize::getCdrSerializedSize(data.dependent_typeids.at(a), current_alignment);}

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (data.continuation_point.size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);




    return current_alignment - initial_alignment;
}

void TypeLookup_getTypeDependencies_Out::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << dependent_typeids;
    scdr << continuation_point;
}

void TypeLookup_getTypeDependencies_Out::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> dependent_typeids;
    dcdr >> continuation_point;
}

TypeLookup_getTypeDependencies_Result::TypeLookup_getTypeDependencies_Result()
{
    m__d = 0 /* TODO DDS_RETCODE_OK */;
}

TypeLookup_getTypeDependencies_Result::~TypeLookup_getTypeDependencies_Result()
{
}

TypeLookup_getTypeDependencies_Result::TypeLookup_getTypeDependencies_Result(const TypeLookup_getTypeDependencies_Result &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        m_result = x.m_result;
        break;
    default:
        break;
    }
}

TypeLookup_getTypeDependencies_Result::TypeLookup_getTypeDependencies_Result(TypeLookup_getTypeDependencies_Result &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        m_result = std::move(x.m_result);
        break;
    default:
        break;
    }
}

TypeLookup_getTypeDependencies_Result& TypeLookup_getTypeDependencies_Result::operator=(const TypeLookup_getTypeDependencies_Result &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        m_result = x.m_result;
        break;
    default:
        break;
    }

    return *this;
}

TypeLookup_getTypeDependencies_Result& TypeLookup_getTypeDependencies_Result::operator=(TypeLookup_getTypeDependencies_Result &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        m_result = std::move(x.m_result);
        break;
    default:
        break;
    }

    return *this;
}

void TypeLookup_getTypeDependencies_Result::_d(int32_t __d)
{
    bool b = false;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        switch(__d)
        {
        case 0 /* TODO DDS_RETCODE_OK */:
            b = true;
            break;
        default:
            break;
        }
        break;
    }

    if(!b)
    {
        throw BadParamException("Discriminator doesn't correspond with the selected union member");
    }

    m__d = __d;
}

int32_t TypeLookup_getTypeDependencies_Result::_d() const
{
    return m__d;
}

int32_t& TypeLookup_getTypeDependencies_Result::_d()
{
    return m__d;
}

void TypeLookup_getTypeDependencies_Result::result(const TypeLookup_getTypeDependencies_Out &_result)
{
    m_result = _result;
    m__d = 0 /* TODO DDS_RETCODE_OK */;
}

void TypeLookup_getTypeDependencies_Result::result(TypeLookup_getTypeDependencies_Out &&_result)
{
    m_result = std::move(_result);
    m__d = 0 /* TODO DDS_RETCODE_OK */;
}

const TypeLookup_getTypeDependencies_Out& TypeLookup_getTypeDependencies_Result::result() const
{
    bool b = false;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_result;
}

TypeLookup_getTypeDependencies_Out& TypeLookup_getTypeDependencies_Result::result()
{
    bool b = false;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_result;
}

// TODO(Ricardo) Review
size_t TypeLookup_getTypeDependencies_Result::getCdrSerializedSize(const TypeLookup_getTypeDependencies_Result& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch(data.m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        current_alignment += TypeLookup_getTypeDependencies_Out::getCdrSerializedSize(data.result(), current_alignment);
        break;
    default:
        break;
    }

    return current_alignment - initial_alignment;
}

void TypeLookup_getTypeDependencies_Result::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        scdr << m_result;
        break;
    default:
        break;
    }
}

void TypeLookup_getTypeDependencies_Result::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m__d;

    switch(m__d)
    {
    case 0 /* TODO DDS_RETCODE_OK */:
        dcdr >> m_result;
        break;
    default:
        break;
    }
}


TypeLookup_Call::TypeLookup_Call()
{
    m__d = TypeLookup_getTypes_Hash;
    // m_getTypes com.eprosima.fastrtps.idl.parser.typecode.StructTypeCode@543788f3

    // m_getTypeDependencies com.eprosima.fastrtps.idl.parser.typecode.StructTypeCode@6d3af739

}

TypeLookup_Call::~TypeLookup_Call()
{
}

TypeLookup_Call::TypeLookup_Call(const TypeLookup_Call &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getTypes = x.m_getTypes;
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = x.m_getTypeDependencies;
        break;
    default:
        break;
    }
}

TypeLookup_Call::TypeLookup_Call(TypeLookup_Call &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getTypes = std::move(x.m_getTypes);
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = std::move(x.m_getTypeDependencies);
        break;
    default:
        break;
    }
}

TypeLookup_Call& TypeLookup_Call::operator=(const TypeLookup_Call &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getTypes = x.m_getTypes;
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = x.m_getTypeDependencies;
        break;
    default:
        break;
    }

    return *this;
}

TypeLookup_Call& TypeLookup_Call::operator=(TypeLookup_Call &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getTypes = std::move(x.m_getTypes);
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = std::move(x.m_getTypeDependencies);
        break;
    default:
        break;
    }

    return *this;
}

void TypeLookup_Call::_d(int32_t __d)
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        switch(__d)
        {
        case TypeLookup_getTypes_Hash:
            b = true;
            break;
        default:
            break;
        }
        break;
    case TypeLookup_getDependencies_Hash:
        switch(__d)
        {
        case TypeLookup_getDependencies_Hash:
            b = true;
            break;
        default:
            break;
        }
        break;
    }

    if(!b)
    {
        throw BadParamException("Discriminator doesn't correspond with the selected union member");
    }

    m__d = __d;
}

int32_t TypeLookup_Call::_d() const
{
    return m__d;
}

int32_t& TypeLookup_Call::_d()
{
    return m__d;
}

void TypeLookup_Call::getTypes(const TypeLookup_getTypes_In &_getTypes)
{
    m_getTypes = _getTypes;
    m__d = TypeLookup_getTypes_Hash;
}

void TypeLookup_Call::getTypes(TypeLookup_getTypes_In &&_getTypes)
{
    m_getTypes = std::move(_getTypes);
    m__d = TypeLookup_getTypes_Hash;
}

const TypeLookup_getTypes_In& TypeLookup_Call::getTypes() const
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypes;
}

TypeLookup_getTypes_In& TypeLookup_Call::getTypes()
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypes;
}
void TypeLookup_Call::getTypeDependencies(const TypeLookup_getTypeDependencies_In &_getTypeDependencies)
{
    m_getTypeDependencies = _getTypeDependencies;
    m__d = TypeLookup_getDependencies_Hash;
}

void TypeLookup_Call::getTypeDependencies(TypeLookup_getTypeDependencies_In &&_getTypeDependencies)
{
    m_getTypeDependencies = std::move(_getTypeDependencies);
    m__d = TypeLookup_getDependencies_Hash;
}

const TypeLookup_getTypeDependencies_In& TypeLookup_Call::getTypeDependencies() const
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getDependencies_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypeDependencies;
}

TypeLookup_getTypeDependencies_In& TypeLookup_Call::getTypeDependencies()
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getDependencies_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypeDependencies;
}

// TODO(Ricardo) Review
size_t TypeLookup_Call::getCdrSerializedSize(const TypeLookup_Call& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch(data.m__d)
    {
    case TypeLookup_getTypes_Hash:
        current_alignment += TypeLookup_getTypes_In::getCdrSerializedSize(data.getTypes(), current_alignment);
        break;
    case TypeLookup_getDependencies_Hash:
        current_alignment += TypeLookup_getTypeDependencies_In::getCdrSerializedSize(data.getTypeDependencies(), current_alignment);
        break;
    default:
        break;
    }

    return current_alignment - initial_alignment;
}

void TypeLookup_Call::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        scdr << m_getTypes;
        break;
    case TypeLookup_getDependencies_Hash:
        scdr << m_getTypeDependencies;
        break;
    default:
        break;
    }
}

void TypeLookup_Call::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        dcdr >> m_getTypes;
        break;
    case TypeLookup_getDependencies_Hash:
        dcdr >> m_getTypeDependencies;
        break;
    default:
        break;
    }
}

size_t TypeLookup_Request::getCdrSerializedSize(const TypeLookup_Request& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += rpc::RequestHeader::getCdrSerializedSize(data.header, current_alignment);
    current_alignment += TypeLookup_Call::getCdrSerializedSize(data.data, current_alignment);

    return current_alignment - initial_alignment;
}

void TypeLookup_Request::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << header;
    scdr << data;
}

void TypeLookup_Request::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> header;
    dcdr >> data;
}

TypeLookup_Return::TypeLookup_Return()
{
    m__d = TypeLookup_getTypes_Hash;
    // m_getType com.eprosima.idl.parser.typecode.UnionTypeCode@69930714

    // m_getTypeDependencies com.eprosima.idl.parser.typecode.UnionTypeCode@7a52f2a2

}

TypeLookup_Return::~TypeLookup_Return()
{
}

TypeLookup_Return::TypeLookup_Return(const TypeLookup_Return &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getType = x.m_getType;
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = x.m_getTypeDependencies;
        break;
    default:
        break;
    }
}

TypeLookup_Return::TypeLookup_Return(TypeLookup_Return &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getType = std::move(x.m_getType);
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = std::move(x.m_getTypeDependencies);
        break;
    default:
        break;
    }
}

TypeLookup_Return& TypeLookup_Return::operator=(const TypeLookup_Return &x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getType = x.m_getType;
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = x.m_getTypeDependencies;
        break;
    default:
        break;
    }

    return *this;
}

TypeLookup_Return& TypeLookup_Return::operator=(TypeLookup_Return &&x)
{
    m__d = x.m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        m_getType = std::move(x.m_getType);
        break;
    case TypeLookup_getDependencies_Hash:
        m_getTypeDependencies = std::move(x.m_getTypeDependencies);
        break;
    default:
        break;
    }

    return *this;
}

void TypeLookup_Return::_d(int32_t __d)
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        switch(__d)
        {
        case TypeLookup_getTypes_Hash:
            b = true;
            break;
        default:
            break;
        }
        break;
    case TypeLookup_getDependencies_Hash:
        switch(__d)
        {
        case TypeLookup_getDependencies_Hash:
            b = true;
            break;
        default:
            break;
        }
        break;
    }

    if(!b)
    {
        throw BadParamException("Discriminator doesn't correspond with the selected union member");
    }

    m__d = __d;
}

int32_t TypeLookup_Return::_d() const
{
    return m__d;
}

int32_t& TypeLookup_Return::_d()
{
    return m__d;
}

void TypeLookup_Return::getType(const TypeLookup_getTypes_Result &_getType)
{
    m_getType = _getType;
    m__d = TypeLookup_getTypes_Hash;
}

void TypeLookup_Return::getType(TypeLookup_getTypes_Result &&_getType)
{
    m_getType = std::move(_getType);
    m__d = TypeLookup_getTypes_Hash;
}

const TypeLookup_getTypes_Result& TypeLookup_Return::getType() const
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getType;
}

TypeLookup_getTypes_Result& TypeLookup_Return::getType()
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getType;
}
void TypeLookup_Return::getTypeDependencies(const TypeLookup_getTypeDependencies_Result &_getTypeDependencies)
{
    m_getTypeDependencies = _getTypeDependencies;
    m__d = TypeLookup_getDependencies_Hash;
}

void TypeLookup_Return::getTypeDependencies(TypeLookup_getTypeDependencies_Result &&_getTypeDependencies)
{
    m_getTypeDependencies = std::move(_getTypeDependencies);
    m__d = TypeLookup_getDependencies_Hash;
}

const TypeLookup_getTypeDependencies_Result& TypeLookup_Return::getTypeDependencies() const
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getDependencies_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypeDependencies;
}

TypeLookup_getTypeDependencies_Result& TypeLookup_Return::getTypeDependencies()
{
    bool b = false;

    switch(m__d)
    {
    case TypeLookup_getDependencies_Hash:
        b = true;
        break;
    default:
        break;
    }
    if(!b)
    {
        throw BadParamException("This member is not been selected");
    }

    return m_getTypeDependencies;
}

// TODO(Ricardo) Review
size_t TypeLookup_Return::getCdrSerializedSize(const TypeLookup_Return& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    switch(data.m__d)
    {
    case TypeLookup_getTypes_Hash:
        current_alignment += TypeLookup_getTypes_Result::getCdrSerializedSize(data.getType(), current_alignment);
        break;
    case TypeLookup_getDependencies_Hash:
        current_alignment += TypeLookup_getTypeDependencies_Result::getCdrSerializedSize(data.getTypeDependencies(), current_alignment);
        break;
    default:
        break;
    }

    return current_alignment - initial_alignment;
}

void TypeLookup_Return::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        scdr << m_getType;
        break;
    case TypeLookup_getDependencies_Hash:
        scdr << m_getTypeDependencies;
        break;
    default:
        break;
    }
}

void TypeLookup_Return::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m__d;

    switch(m__d)
    {
    case TypeLookup_getTypes_Hash:
        dcdr >> m_getType;
        break;
    case TypeLookup_getDependencies_Hash:
        dcdr >> m_getTypeDependencies;
        break;
    default:
        break;
    }
}

size_t TypeLookup_Reply::getCdrSerializedSize(const TypeLookup_Reply& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += rpc::RequestHeader::getCdrSerializedSize(data.header, current_alignment);
    current_alignment += TypeLookup_Return::getCdrSerializedSize(data.return_value, current_alignment);

    return current_alignment - initial_alignment;
}

void TypeLookup_Reply::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << header;
    scdr << return_value;
}

void TypeLookup_Reply::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> header;
    dcdr >> return_value;
}

// TypeSupports
bool TypeLookup_RequestTypeSupport::serialize(
        void* data,
        fastrtps::rtps::SerializedPayload_t* payload)
{
    TypeLookup_Request* type = static_cast<TypeLookup_Request*>(data);
    //eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        type->serialize(ser); // Serialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength()); 	//Get the serialized length
    return true;
}

bool TypeLookup_RequestTypeSupport::deserialize(
        fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    TypeLookup_Request* p_type = static_cast<TypeLookup_Request*>(data); 	//Convert DATA to pointer of your type
    //eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length); 	// Object that manages the raw buffer.
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        p_type->deserialize(deser); //Deserialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

size_t TypeLookup_RequestTypeSupport::getCdrSerializedSize(
        const TypeLookup_Request& data,
        size_t current_alignment)
{
    return TypeLookup_Request::getCdrSerializedSize(data, current_alignment);
}

void* TypeLookup_RequestTypeSupport::create_data()
{
    return new TypeLookup_Request();
}

void TypeLookup_RequestTypeSupport::delete_data(
        void* data)
{
    delete static_cast<TypeLookup_Request*>(data);
}

/*
TypeLookup_ReplyPubSubType::TypeLookup_ReplyPubSubType()
{

}

TypeLookup_ReplyPubSubType::~TypeLookup_ReplyPubSubType()
{

}

bool TypeLookup_ReplyPubSubType::serialize(
        void* data,
        fastrtps::rtps::SerializedPayload_t* payload)
{

}

bool TypeLookup_ReplyPubSubType::deserialize(
        fastrtps::rtps::SerializedPayload_t *payload,
        void *data)
{

}

TypeLookup_ReplyTypeSupport::TypeLookup_ReplyTypeSupport()
    : TypeSupport(new TypeLookup_Reply())
{
}
*/
bool TypeLookup_ReplyTypeSupport::serialize(
        void* data,
        fastrtps::rtps::SerializedPayload_t* payload)
{
    TypeLookup_Reply* type = static_cast<TypeLookup_Reply*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        type->serialize(ser); // Serialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength()); 	//Get the serialized length
    return true;
}

bool TypeLookup_ReplyTypeSupport::deserialize(
        fastrtps::rtps::SerializedPayload_t* payload,
        void* data)
{
    TypeLookup_Reply* p_type = static_cast<TypeLookup_Reply*>(data); 	//Convert DATA to pointer of your type
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length); 	// Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        p_type->deserialize(deser); //Deserialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

size_t TypeLookup_ReplyTypeSupport::getCdrSerializedSize(
        const TypeLookup_Reply& data,
        size_t current_alignment)
{
    return TypeLookup_Reply::getCdrSerializedSize(data, current_alignment);
}

void* TypeLookup_ReplyTypeSupport::create_data()
{
    return new TypeLookup_Reply();
}

void TypeLookup_ReplyTypeSupport::delete_data(
        void* data)
{
    delete static_cast<TypeLookup_Reply*>(data);
}
