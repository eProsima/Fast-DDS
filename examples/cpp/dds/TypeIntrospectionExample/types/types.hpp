// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file types.hpp
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_HPP_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_HPP_

#include <memory>
#include <ostream>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataPtr.h>
#include <fastrtps/types/DynamicTypeMember.h>

enum class DataTypeKind
{
    HELLO_WORLD,
    ARRAY,
    SEQUENCE,
    STRUCT,
    PLAIN,
    SIMPLELARGE,
    KEY,
    COMPLEX_ARRAY,
    COMPLEX_SEQUENCE,
    SUPER_COMPLEX,
};

enum class GeneratorKind
{
    GEN,
    XML,
    CODE
};

constexpr const char* HELLO_WORLD_DATA_TYPE_NAME = "HelloWorld_TypeIntrospectionExample";
constexpr const char* ARRAY_DATA_TYPE_NAME = "Array_TypeIntrospectionExample";
constexpr const char* SEQUENCE_DATA_TYPE_NAME = "Sequence_TypeIntrospectionExample";
constexpr const char* STRUCT_DATA_TYPE_NAME = "Struct_TypeIntrospectionExample";
constexpr const char* PLAIN_DATA_TYPE_NAME = "Plain_TypeIntrospectionExample";
constexpr const char* SIMPLELARGE_DATA_TYPE_NAME = "SimpleLarge_TypeIntrospectionExample";
constexpr const char* KEY_DATA_TYPE_NAME = "Key_TypeIntrospectionExample";
constexpr const char* COMPLEX_ARRAY_DATA_TYPE_NAME = "ComplexArray_TypeIntrospectionExample";
constexpr const char* COMPLEX_SEQUENCE_DATA_TYPE_NAME = "ComplexSequence_TypeIntrospectionExample";
constexpr const char* SUPER_COMPLEX_DATA_TYPE_NAME = "SuperComplex_TypeIntrospectionExample";

class IDataType
{
public:

    virtual ~IDataType() = default;

    virtual std::string name() const = 0;

    virtual std::string xml() const = 0;

    virtual eprosima::fastrtps::types::DynamicData_ptr get_data(
            const unsigned int& index) const = 0;

    virtual eprosima::fastrtps::types::DynamicType_ptr get_type() const = 0;
};

template <DataTypeKind Data, GeneratorKind Gen>
class DataType : public IDataType
{
public:

    DataType();

    virtual std::string name() const override;

    virtual std::string xml() const override;

    virtual eprosima::fastrtps::types::DynamicData_ptr get_data(
            const unsigned int& index) const override;

    virtual eprosima::fastrtps::types::DynamicType_ptr get_type() const override;

protected:

    virtual eprosima::fastrtps::types::DynamicType_ptr generate_type_() const;

    eprosima::fastrtps::types::DynamicType_ptr dyn_type_;
};

template <DataTypeKind Data>
eprosima::fastrtps::types::DynamicData_ptr get_data_by_type(
        const unsigned int& index,
        eprosima::fastrtps::types::DynamicType_ptr dyn_type);

std::unique_ptr<IDataType> data_type_factory(
        const DataTypeKind data_kind,
        const GeneratorKind gen_kind);


///////////////////////////////////////////
// Dynamic Types auxiliary functions
///////////////////////////////////////////

bool is_basic_kind(
        const eprosima::fastrtps::types::TypeKind kind);

eprosima::fastrtps::types::DynamicType_ptr internal_array_type(
        const eprosima::fastrtps::types::DynamicType_ptr& type);

///////////////////////////////////////////
// Serialization operators
///////////////////////////////////////////

std::ostream& operator <<(
        std::ostream& output,
        const GeneratorKind& gen);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastrtps::types::DynamicData_ptr& data);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastrtps::types::DynamicType_ptr& type);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastrtps::types::TypeKind& kind);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastrtps::types::DynamicTypeMember* member);

// Include implementation template file
#include "types.ipp"

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_HPP_ */
