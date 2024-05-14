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

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_IPP_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_IPP_

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>

template <DataTypeKind Data, GeneratorKind Gen>
DataType<Data, Gen>::DataType()
{
    switch (Gen)
    {
        case GeneratorKind::GEN:
            dynamic_ = false;
            break;

        case GeneratorKind::GEN_DYN:
        case GeneratorKind::CODE:
        case GeneratorKind::XML:
        default:
            dynamic_ = true;
            break;
    }

    generate_type_support_();
}

template <DataTypeKind Data, GeneratorKind Gen>
bool DataType<Data, Gen>::dynamic() const
{
    return dynamic_;
}

template <DataTypeKind Data, GeneratorKind Gen>
std::string DataType<Data, Gen>::name() const
{
    // This method is generic to avoid implementing the same code for each Generator
    // because C++ does not allow partial specialization of methods.
    switch (Data)
    {
        case DataTypeKind::HELLO_WORLD:
            return HELLO_WORLD_DATA_TYPE_NAME;

        case DataTypeKind::ARRAY:
            return ARRAY_DATA_TYPE_NAME;

        case DataTypeKind::SEQUENCE:
            return SEQUENCE_DATA_TYPE_NAME;

        case DataTypeKind::STRUCT:
            return STRUCT_DATA_TYPE_NAME;

        case DataTypeKind::PLAIN:
            return PLAIN_DATA_TYPE_NAME;

        case DataTypeKind::SIMPLELARGE:
            return SIMPLELARGE_DATA_TYPE_NAME;

        case DataTypeKind::KEY:
            return KEY_DATA_TYPE_NAME;

        case DataTypeKind::COMPLEX_ARRAY:
            return COMPLEX_ARRAY_DATA_TYPE_NAME;

        case DataTypeKind::COMPLEX_SEQUENCE:
            return COMPLEX_SEQUENCE_DATA_TYPE_NAME;

        case DataTypeKind::SUPER_COMPLEX:
            return SUPER_COMPLEX_DATA_TYPE_NAME;

        case DataTypeKind::DATA_TEST:
            return DATA_TEST_DATA_TYPE_NAME;
            // return "AllStruct_TypeIntrospectionExample";

        default:
            throw std::runtime_error("Unsupported data type");
    }
}

template <DataTypeKind Data, GeneratorKind Gen>
std::string DataType<Data, Gen>::xml() const
{
    // This method is generic to avoid implementing the same code for each Generator
    // because C++ does not allow partial specialization of methods.
    switch (Data)
    {
        case DataTypeKind::HELLO_WORLD:
            return "xml/HelloWorldXml.xml";
        case DataTypeKind::ARRAY:
            return "xml/ArrayXml.xml";
        case DataTypeKind::SEQUENCE:
            return "xml/SequenceXml.xml";
        case DataTypeKind::STRUCT:
            return "xml/StructXml.xml";
        case DataTypeKind::PLAIN:
            return "xml/PlainXml.xml";
        case DataTypeKind::SIMPLELARGE:
            return "xml/SimpleLargeXml.xml";
        case DataTypeKind::KEY:
            return "xml/KeyXml.xml";
        case DataTypeKind::COMPLEX_ARRAY:
            return "xml/ComplexArrayXml.xml";
        case DataTypeKind::COMPLEX_SEQUENCE:
            return "xml/ComplexSequenceXml.xml";
        case DataTypeKind::SUPER_COMPLEX:
            return "xml/SuperComplexXml.xml";
        case DataTypeKind::DATA_TEST:
            return "xml/DataTestXml.xml";
        default:
            throw std::runtime_error("Unsupported data type");
    }
}

template <DataTypeKind Data, GeneratorKind Gen>
void DataType<Data, Gen>::register_type_object_representation() const
{
    register_type_object_representation_gen<Data>();
}

template <DataTypeKind Data, GeneratorKind Gen>
void* DataType<Data, Gen>::get_data(
        const unsigned int& index) const
{
    if (dynamic_)
    {
        return get_dynamic_data_by_type_support<Data>(index, type_support_);
    }
    else
    {
        return get_data_by_type_support<Data>(index, type_support_);
    }
}

template <DataTypeKind Data, GeneratorKind Gen>
eprosima::fastdds::dds::TypeSupport DataType<Data, Gen>::get_type_support() const
{
    return type_support_;
}

template <DataTypeKind Data, GeneratorKind Gen>
void DataType<Data, Gen>::generate_type_support_xml_()
{
    using namespace eprosima::fastdds::dds;

    auto participant_factory = DomainParticipantFactory::get_instance();

    if (RETCODE_OK !=
            participant_factory->load_XML_profiles_file(xml()))
    {
        throw std::ios_base::failure(
                  "Cannot open XML file. Please, run the publisher from the folder that contains this XML file.");
    }

    // Create Dynamic type
    traits<DynamicType>::ref_type dynamic_type;
    if (RETCODE_OK !=
            participant_factory->get_dynamic_type_builder_from_xml_by_name(name(), dynamic_type))
    {
        throw std::ios_base::failure(
                  "Failed to create dynamic type from XML file.");
    }

    type_support_.reset(new DynamicPubSubType(dynamic_type));
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPES_TYPES_IPP_ */
