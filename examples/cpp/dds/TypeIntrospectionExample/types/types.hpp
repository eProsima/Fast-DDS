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

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

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
    DATA_TEST,
};

enum class GeneratorKind
{
    GEN,
    GEN_DYN,
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
constexpr const char* DATA_TEST_DATA_TYPE_NAME = "DataTest_TypeIntrospectionExample";

class IDataType
{
public:

    virtual ~IDataType() = default;

    virtual bool dynamic() const = 0;

    virtual std::string name() const = 0;

    virtual std::string xml() const = 0;

    virtual void register_type_object_representation() const = 0;

    virtual void* get_data(
            const unsigned int& index) const = 0;

    virtual eprosima::fastdds::dds::TypeSupport get_type_support() const = 0;
};

template <DataTypeKind Data, GeneratorKind Gen>
class DataType : public IDataType
{
public:

    DataType();

    virtual bool dynamic() const override;

    virtual std::string name() const override;

    virtual std::string xml() const override;

    virtual void register_type_object_representation() const override;

    virtual void* get_data(
            const unsigned int& index) const override;

    virtual eprosima::fastdds::dds::TypeSupport get_type_support() const override;

protected:

    void generate_type_support_();

    void generate_type_support_xml_();

    eprosima::fastdds::dds::TypeSupport type_support_;

    bool dynamic_;
};

template <DataTypeKind Data>
void* get_dynamic_data_by_type_support(
        const unsigned int& index,
        eprosima::fastdds::dds::TypeSupport type_support);

template <DataTypeKind Data>
void* get_data_by_type_support(
        const unsigned int& index,
        eprosima::fastdds::dds::TypeSupport type_support);

template <DataTypeKind Data>
void register_type_object_representation_gen();

std::unique_ptr<IDataType> data_type_factory(
        const DataTypeKind data_kind,
        const GeneratorKind gen_kind);

///////////////////////////////////////////
// Dynamic Types auxiliary functions
///////////////////////////////////////////

bool is_basic_kind(
        const eprosima::fastdds::dds::TypeKind kind);

eprosima::fastdds::dds::DynamicType::_ref_type internal_array_type(
        const eprosima::fastdds::dds::DynamicType::_ref_type& type);

///////////////////////////////////////////
// Serialization operators
///////////////////////////////////////////

std::ostream& operator <<(
        std::ostream& output,
        const GeneratorKind& gen);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastdds::dds::DynamicData::_ref_type& data);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastdds::dds::DynamicType::_ref_type& type);

std::ostream& operator <<(
        std::ostream& output,
        const eprosima::fastdds::dds::TypeKind& kind);

// std::ostream& operator <<(
//         std::ostream& output,
//         const eprosima::fastrtps::types::DynamicTypeMember* member);

// Include implementation template file
#include "types.ipp"

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_HPP_ */
