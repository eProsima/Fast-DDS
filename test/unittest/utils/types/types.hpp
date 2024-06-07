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
    DATA_TEST,
};

constexpr const char* HELLO_WORLD_DATA_TYPE_NAME = "HelloWorld_TypeIntrospectionExample";
constexpr const char* DATA_TEST_DATA_TYPE_NAME = "DataTest_TypeIntrospectionExample";

class IDataType
{
public:

    virtual ~IDataType() = default;

    virtual std::string name() const = 0;

    virtual void register_type_object_representation() const = 0;

    virtual eprosima::fastdds::dds::DynamicType::_ref_type generate_type_support_and_get_dyn_type() const = 0;
};

template <DataTypeKind Data>
class DataType : public IDataType
{
public:

    DataType();

    virtual std::string name() const override;

    virtual void register_type_object_representation() const override;

    virtual eprosima::fastdds::dds::DynamicType::_ref_type generate_type_support_and_get_dyn_type() const override;

protected:

    void generate_type_support_();
};

template <DataTypeKind Data>
void register_type_object_representation_gen();

std::unique_ptr<IDataType> data_type_factory(
        const DataTypeKind data_kind);

///////////////////////////////////////////
// Dynamic Types auxiliary functions
///////////////////////////////////////////

bool is_basic_kind(
        const eprosima::fastdds::dds::TypeKind kind);

eprosima::fastdds::dds::DynamicType::_ref_type internal_array_type(
        const eprosima::fastdds::dds::DynamicType::_ref_type& type);

// Include implementation template file
#include "types.ipp"

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_HPP_ */
