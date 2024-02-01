// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//

#include <cstdlib>
#include <iostream>
#include <tinyxml2.h>

#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLTree.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

#include "../../fastdds/xtypes/dynamic_types/AnnotationDescriptorImpl.hpp"
#include "../../fastdds/xtypes/dynamic_types/MemberDescriptorImpl.hpp"
#include "../../fastdds/xtypes/dynamic_types/TypeDescriptorImpl.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::xmlparser;

XMLP_ret XMLParser::parseXMLDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:group name="moduleElems">
            <xs:sequence>
                <xs:choice maxOccurs="unbounded">
                    <xs:element name="struct" type="structDcl" minOccurs="0"/>
                    <xs:element name="union" type="unionDcl" minOccurs="0"/>
                    <xs:element name="enum" type="enumDcl" minOccurs="0"/>
                    <xs:element name="typedef" type="typedefDcl" minOccurs="0"/>
                    <xs:element name="bitset" type="bitsetDcl" minOccurs="0"/>
                    <xs:element name="bitmask" type="bitmaskDcl" minOccurs="0"/>
                </xs:choice>
            </xs:sequence>
        </xs:group>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        const std::string type = p_aux0->Value();
        if (type.compare(STRUCT) == 0)
        {
            ret = parseXMLStructDynamicType(p_aux0);
        }
        else if (type.compare(UNION) == 0)
        {
            ret = parseXMLUnionDynamicType(p_aux0);
        }
        else if (type.compare(ENUM) == 0)
        {
            ret = parseXMLEnumDynamicType(p_aux0);
        }
        else if (type.compare(TYPEDEF) == 0)
        {
            ret = parseXMLAliasDynamicType(p_aux0);
        }
        else if (type.compare(BITSET) == 0)
        {
            ret = parseXMLBitsetDynamicType(p_aux0);
        }
        else if (type.compare(BITMASK) == 0)
        {
            ret = parseXMLBitmaskDynamicType(p_aux0);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing type: Type " << type << " not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }

        if (ret != XMLP_ret::XML_OK)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing type " << type << ".");
            break;
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTypes(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:element name="types">
            <xs:complexType>
                <xs:group ref="moduleElems"/>
            </xs:complexType>
        </xs:element>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_aux0 = nullptr, * p_aux1 = nullptr;
    p_aux0 = p_root->FirstChildElement(TYPES);
    if (p_aux0 != nullptr)
    {
        const char* name = nullptr;
        for (p_aux1 = p_aux0->FirstChildElement(); p_aux1 != nullptr; p_aux1 = p_aux1->NextSiblingElement())
        {
            name = p_aux1->Name();
            if (strcmp(name, TYPE) == 0)
            {
                if (XMLP_ret::XML_OK != parseXMLDynamicType(p_aux1))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'types'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    else // Directly root is TYPES?
    {
        const char* name = nullptr;
        for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
        {
            name = p_aux0->Name();
            if (strcmp(name, TYPE) == 0)
            {
                if (XMLP_ret::XML_OK != parseXMLDynamicType(p_aux0))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'types'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLBitvalueDynamicType(
        tinyxml2::XMLElement* p_root,
        fastdds::dds::traits<fastdds::dds::DynamicTypeBuilder>::ref_type builder,
        uint16_t& field_position)
{
    /*
        <xs:complexType name="bit_valueType">
            <xs:attribute name="name" type="stringType" use="required"/>
            <xs:attribute name="position" type="int16Type" use="optional"/>
        </xs:complexType>
     */
    if (p_root == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing bitmask: Node not found.");
        return XMLP_ret::XML_ERROR;
    }

    const char* memberName = p_root->Attribute(NAME);
    const char* position = p_root->Attribute(POSITION);

    if (position != nullptr)
    {
        try
        {
            field_position = static_cast<uint16_t>(std::stoul(position));
        }
        catch (const std::exception&)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing bit_value position: Invalid (must be an unsigned short).");
            return XMLP_ret::XML_ERROR;
        }
    }

    if (memberName == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing bit_value name: Not found.");
        return XMLP_ret::XML_ERROR;
    }

    traits<MemberDescriptorImpl>::ref_type md {std::make_shared<MemberDescriptorImpl>()};
    md->index(field_position);
    md->name(memberName);
    md->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    builder->add_member(md);
    //TODO(richiware) get_builtin_annotation
    //builder.apply_annotation_to_member(
    //    builder.get_member_id_by_name(memberName), ANNOTATION_POSITION_ID, "value", position);
    ++field_position;

    return XMLP_ret::XML_OK;
}

static traits<DynamicType>::ref_type getDiscriminatorTypeBuilder(
        const std::string& disc,
        uint32_t bound = 0)
{
    /*
       mKind == TK_BOOLEAN || mKind == TK_BYTE || mKind == TK_INT16 || mKind == TK_INT32 ||
        mKind == TK_INT64 || mKind == TK_UINT16 || mKind == TK_UINT32 || mKind == TK_UINT64 ||
        mKind == TK_FLOAT32 || mKind == TK_FLOAT64 || mKind == TK_FLOAT128 || mKind == TK_CHAR8 ||
        mKind == TK_CHAR16 || mKind == TK_STRING8 || mKind == TK_STRING16 || mKind == TK_ENUM || mKind == TK_BITMASK
     */
    auto factory = DynamicTypeBuilderFactory::get_instance();
    if (0 == disc.compare(BOOLEAN))
    {
        return factory->get_primitive_type(TK_BOOLEAN);
    }
    else if (0 == disc.compare(TBYTE) ||
            0 == disc.compare(OCTET))
    {
        return factory->get_primitive_type(TK_BYTE);
    }
    else if (0 == disc.compare(INT8))
    {
        return factory->get_primitive_type(TK_INT8);
    }
    else if (0 == disc.compare(UINT8))
    {
        return factory->get_primitive_type(TK_UINT8);
    }
    else if (0 == disc.compare(SHORT))
    {
        return factory->get_primitive_type(TK_INT16);
    }
    else if (0 == disc.compare(LONG))
    {
        return factory->get_primitive_type(TK_INT32);
    }
    else if (0 == disc.compare(LONGLONG))
    {
        return factory->get_primitive_type(TK_INT64);
    }
    else if (0 == disc.compare(USHORT))
    {
        return factory->get_primitive_type(TK_UINT16);
    }
    else if (0 == disc.compare(ULONG))
    {
        return factory->get_primitive_type(TK_UINT32);
    }
    else if (0 == disc.compare(ULONGLONG))
    {
        return factory->get_primitive_type(TK_UINT64);
    }
    else if (0 == disc.compare(FLOAT))
    {
        return factory->get_primitive_type(TK_FLOAT32);
    }
    else if (0 == disc.compare(DOUBLE))
    {
        return factory->get_primitive_type(TK_FLOAT64);
    }
    else if (0 == disc.compare(LONGDOUBLE))
    {
        return factory->get_primitive_type(TK_FLOAT128);
    }
    else if (0 == disc.compare(CHAR))
    {
        return factory->get_primitive_type(TK_CHAR8);
    }
    else if (0 == disc.compare(WCHAR))
    {
        return factory->get_primitive_type(TK_CHAR16);
    }
    else if (0 == disc.compare(STRING))
    {
        return factory->create_string_type(0 == bound ? LENGTH_UNLIMITED : bound)->build();
    }
    else if (0 == disc.compare(WSTRING))
    {
        return factory->create_wstring_type(0 == bound ? LENGTH_UNLIMITED : bound)->build();
    }

    eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type ret;
    XMLProfileManager::getDynamicTypeByName(ret, disc);
    return ret;
}

XMLP_ret XMLParser::parseXMLAliasDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <typedef name="MyAliasEnum" type="nonBasic" nonBasicTypeName="MyEnum"/>

        <typedef name="MyArray" type="int32" arrayDimensions="2,2"/>

        <xs:complexType name="typedefDcl">
            <xs:attribute name="name" type="identifierName" use="required"/>
            <xs:attribute name="type" type="string" use="required"/>
            <xs:attribute name="key_type" type="string" use="optional"/>
            <xs:attribute name="arrayDimensions" type="string" use="optional"/>
            <xs:attribute name="nonBasicTypeName" type="string" use="optional"/>
            <xs:attribute name="sequenceMaxLength" type="string" use="optional"/>
            <xs:attribute name="mapMaxLength" type="string" use="optional"/>
        </xs:complexType>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;

    const char* type = p_root->Attribute(TYPE);
    if (type != nullptr)
    {
        if (strcmp(type, NON_BASIC_TYPE) == 0)
        {
            const char* typeNonBasicName = p_root->Attribute(NON_BASIC_TYPE_NAME);
            if (typeNonBasicName != nullptr)
            {
                type = typeNonBasicName;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member type: Not found.");
                ret = XMLP_ret::XML_ERROR;
            }
        }

        traits<DynamicType>::ref_type value_type;
        if ((p_root->Attribute(ARRAY_DIMENSIONS) != nullptr) ||
                (p_root->Attribute(SEQ_MAXLENGTH) != nullptr) ||
                (p_root->Attribute(MAP_MAXLENGTH) != nullptr))
        {
            value_type = parseXMLMemberDynamicType(p_root);
        }
        else
        {
            uint32_t bound = 0;
            const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
            if (boundStr != nullptr)
            {
                bound = static_cast<uint32_t>(std::atoi(boundStr));
            }
            value_type = getDiscriminatorTypeBuilder(type, bound);
        }

        if (value_type)
        {
            const char* name = p_root->Attribute(NAME);
            if (name != nullptr)
            {

                traits<TypeDescriptorImpl>::ref_type alias_descriptor {std::make_shared<TypeDescriptorImpl>(TK_ALIAS,
                                                                               name)};
                alias_descriptor->base_type(value_type);
                traits<DynamicTypeBuilder>::ref_type builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                                  alias_descriptor)};
                XMLProfileManager::insertDynamicTypeByName(name, builder->build());
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing alias type: No name attribute given.");
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing alias type: Value not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing alias type: Type not defined.");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLBitsetDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <bitset name="MyBitSet">
            <bitfield name="a" bit_bound="3"/>
            <bitfield name="b" bit_bound="1"/>
            <bitfield bit_bound="4"/>
            <bitfield name="c" bit_bound="10"/>
            <bitfield name="d" bit_bound="12" type="short"/>
        </bitset>

        <xs:complexType name="bitsetDcl">
            <xs:sequence>
                <xs:choice maxOccurs="unbounded">
                    <xs:element name="bitfield" type="bitfieldDcl" minOccurs="1"/>
                </xs:choice>
            </xs:sequence>
            <xs:attribute name="name" type="stringType" use="required"/>
            <xs:attribute name="baseType" type="stringType" use="optional"/>
        </xs:complexType>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    MemberId mId{0};

    const char* name = p_root->Attribute(NAME);
    if (nullptr == name)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing 'bitsetDcl' type. No name attribute given.");
        return XMLP_ret::XML_ERROR;
    }

    traits<TypeDescriptorImpl>::ref_type bitset_descriptor {std::make_shared<TypeDescriptorImpl>(TK_BITSET, name)};

    const char* baseType = p_root->Attribute(BASE_TYPE);
    if (baseType != nullptr)
    {
        DynamicType::_ref_type parent_type;
        XMLProfileManager::getDynamicTypeByName(parent_type, baseType);
        if (parent_type && TK_BITSET == parent_type->get_kind())
        {
            bitset_descriptor->base_type(parent_type);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid baseType found into 'bitsetDcl'. Name: " << baseType);
            return XMLP_ret::XML_ERROR;
        }
    }

    traits<DynamicTypeBuilder>::ref_type type_builder =
            DynamicTypeBuilderFactory::get_instance()->create_type(bitset_descriptor);

    const char* element_name = nullptr;
    uint16_t position = 0;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, BITFIELD) == 0)
        {
            if (!parseXMLBitfieldDynamicType(p_element, type_builder, mId++, position))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'bitsetDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, type_builder->build());
    return ret;
}

traits<eprosima::fastdds::dds::DynamicType>::ref_type XMLParser::parseXMLBitfieldDynamicType(
        tinyxml2::XMLElement* p_root,
        traits<fastdds::dds::DynamicTypeBuilder>::ref_type builder,
        MemberId mId,
        uint16_t& position)
{
    /*
        <xs:complexType name="bitfieldDcl">
            <xs:attribute name="name" type="stringType" use="optional"/>
            <xs:attribute name="type" type="stringType" use="optional"/>
            <xs:attribute name="bit_bound" type="int16Type" use="required"/>
        </xs:complexType>
     */
    if (p_root == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing bitfield: Node not found.");
        return {};
    }

    const char* memberType = p_root->Attribute(TYPE);
    const char* memberName = p_root->Attribute(NAME);
    const char* bit_bound = p_root->Attribute(BIT_BOUND);

    if (bit_bound == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing bitfield bit_bound: Not found.");
        return {};
    }

    if (memberName == nullptr)
    {
        memberName = "";
    }

    traits<eprosima::fastdds::dds::DynamicType>::ref_type member_type;
    traits<DynamicTypeBuilderFactory>::ref_type factory = DynamicTypeBuilderFactory::get_instance();

    if (memberType == nullptr)
    {
        try
        {
            uint16_t size = static_cast<uint16_t>(std::stoul(bit_bound));
            if (size == 1)
            {
                memberType = BOOLEAN;
            }
            else if (size <= 8)
            {
                memberType = CHAR;
            }
            else if (size <= 16)
            {
                memberType = USHORT;
            }
            else if (size <= 32)
            {
                memberType = ULONG;
            }
            else if (size <= 64)
            {
                memberType = ULONGLONG;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed creating bitfield, size too big: " << bit_bound);
                return {};
            }
        }
        catch (...)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Failed creating bitfield, invalid bit_bound (must be an unsigned short): "
                    << bit_bound);
            return {};
        }
    }

    if (0 == strncmp(memberType, BOOLEAN, 8))
    {
        member_type = factory->get_primitive_type(TK_BOOLEAN);
    }
    else if (0 == strncmp(memberType, CHAR, 5))
    {
        member_type = factory->get_primitive_type(TK_CHAR8);
    }
    else if (0 == strncmp(memberType, WCHAR, 6))
    {
        member_type = factory->get_primitive_type(TK_CHAR16);
    }
    else if (0 == strncmp(memberType, TBYTE, 6)
            || 0 == strncmp(memberType, OCTET, 6))
    {
        member_type = factory->get_primitive_type(TK_BYTE);
    }
    else if (0 == strncmp(memberType, UINT8, 6))
    {
        member_type = factory->get_primitive_type(TK_INT8);
    }
    else if (0 == strncmp(memberType, INT8, 5))
    {
        member_type = factory->get_primitive_type(TK_UINT8);
    }
    else if (0 == strncmp(memberType, SHORT, 6))
    {
        member_type = factory->get_primitive_type(TK_INT16);
    }
    else if (0 == strncmp(memberType, LONG, 5))
    {
        member_type = factory->get_primitive_type(TK_INT32);
    }
    else if (0 == strncmp(memberType, ULONG, 13))
    {
        member_type = factory->get_primitive_type(TK_UINT32);
    }
    else if (0 == strncmp(memberType, USHORT, 14))
    {
        member_type = factory->get_primitive_type(TK_UINT16);
    }
    else if (0 == strncmp(memberType, LONGLONG, 9))
    {
        member_type = factory->get_primitive_type(TK_INT64);
    }
    else if (0 == strncmp(memberType, ULONGLONG, 17))
    {
        member_type = factory->get_primitive_type(TK_UINT64);
    }
    else // Unsupported type?
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Failed creating bitfield " << memberName << ": Type " << memberType << " unsupported.");
    }

    if (!member_type)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Failed creating " << memberType << ": " << memberName);
    }

    traits<MemberDescriptorImpl>::ref_type md {std::make_shared<MemberDescriptorImpl>()};
    md->id(mId);
    md->name(memberName);
    md->type(member_type);
    builder->add_member(md);

    if (!std::string(memberName).empty())
    {
        /*!TODO(richiware) get_builtin_annotation.
           traits<AnnotationDescriptorImpl>::ref_type ad = std::make_shared<AnnotationDescriptorImpl>();
           ad.set_type(fact.create_annotation_primitive(ANNOTATION_BIT_BOUND_ID));
           ad.set_value("value", bit_bound);
           builder.apply_annotation_to_member(mId, ad);

           ad.set_type(fact.create_annotation_primitive(ANNOTATION_POSITION_ID));
           ad.set_value("value", std::to_string(position).c_str());
           builder.apply_annotation_to_member(mId, ad);
         */
    }
    position += static_cast<uint16_t>(atoi(bit_bound));

    return member_type;
}

XMLP_ret XMLParser::parseXMLBitmaskDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <bitmask name="MyBitMask" bit_bound="8">
            <bit_value name="flag0" position="0"/>
            <bit_value name="flag1"/>
            <bit_value name="flag2" position="2"/>
            <bit_value name="flag5" position="5"/>
        </bitmask>

        <xs:complexType name="bitmaskDcl">
            <xs:sequence>
                <xs:element name="bit_value" type="bit_valueType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
            <xs:attribute name="name" use="required"/>
            <xs:attribute name="bit_bound" use="optional"/>
        </xs:complexType>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    uint16_t bit_bound = 32;
    const char* anno_bit_bound = p_root->Attribute(BIT_BOUND);
    if (anno_bit_bound != nullptr)
    {
        auto input_bit_bound = std::atoi(anno_bit_bound);
        if (input_bit_bound < 1 || input_bit_bound > 64)
        {
            return XMLP_ret::XML_ERROR;
        }
        bit_bound = static_cast<uint16_t>(input_bit_bound);
    }

    const char* name = p_root->Attribute(NAME);
    if (nullptr == name || name[0] == '\0')
    {
        return XMLP_ret::XML_ERROR;
    }
    traits<TypeDescriptorImpl>::ref_type bitmask_descriptor {std::make_shared<TypeDescriptorImpl>(TK_BITMASK, name)};
    bitmask_descriptor->element_type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_BOOLEAN));
    bitmask_descriptor->bound().push_back(bit_bound);
    traits<DynamicTypeBuilder>::ref_type type_builder {
        DynamicTypeBuilderFactory::get_instance()->create_type(bitmask_descriptor)};
    uint16_t position = 0;

    const char* element_name = nullptr;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, BIT_VALUE) == 0)
        {
            if (parseXMLBitvalueDynamicType(p_element, type_builder, position) != XMLP_ret::XML_OK)
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'bitmaskDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, type_builder->build());
    return ret;
}

XMLP_ret XMLParser::parseXMLEnumDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="enumeratorType">
            <xs:attribute name="name" type="stringType" use="required"/>
            <xs:attribute name="value" type="stringType" use="optional"/>
        </xs:complexType>

        <xs:complexType name="enum">
            <xs:attribute name="name" use="required"/>
            <xs:sequence>
                <xs:element name="enumerator" type="enumeratorType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>

        //TODO: Enum bitbound to set the internal field
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* enumName = p_root->Attribute(NAME);

    if (enumName == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing 'enum' type. No name attribute given.");
        return XMLP_ret::XML_ERROR;
    }


    traits<TypeDescriptorImpl>::ref_type enum_descriptor {std::make_shared<TypeDescriptorImpl>(TK_ENUM, enumName)};
    traits<DynamicTypeBuilder>::ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->create_type(
                                                           enum_descriptor)};

    for (tinyxml2::XMLElement* literal = p_root->FirstChildElement(ENUMERATOR);
            literal != nullptr; literal = literal->NextSiblingElement(ENUMERATOR))
    {
        const char* name = literal->Attribute(NAME);
        if (name == nullptr)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing enum type: Literals must have name.");
            return XMLP_ret::XML_ERROR;
        }

        /* TODO(richiware) Change to set annotation @value
           const char* value = literal->Attribute(VALUE);
           if (value != nullptr)
           {
            currValue = static_cast<uint32_t>(std::atoi(value));
           }
         */

        traits<MemberDescriptorImpl>::ref_type md {std::make_shared<MemberDescriptorImpl>()};
        md->type(DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT32));
        md->name(name);
        type_builder->add_member(md);
    }

    XMLProfileManager::insertDynamicTypeByName(enumName, type_builder->build());
    return ret;
}

XMLP_ret XMLParser::parseXMLStructDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="structDcl">
            <xs:sequence>
                <xs:choice maxOccurs="unbounded">
                    <xs:element name="member" type="memberDcl" minOccurs="1"/>
                </xs:choice>
            </xs:sequence>
            <xs:attribute name="name" type="string" use="required"/>
            <xs:attribute name="baseType" type="stringType" use="optional"/>
        </xs:complexType>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* name = p_root->Attribute(NAME);
    MemberId mId{0};

    traits<TypeDescriptorImpl>::ref_type structure_descriptor {std::make_shared<TypeDescriptorImpl>(TK_STRUCTURE,
                                                                       name)};

    const char* baseType = p_root->Attribute(BASE_TYPE);
    if (baseType != nullptr)
    {
        DynamicType::_ref_type parent_type;
        XMLProfileManager::getDynamicTypeByName(parent_type, baseType);
        if (parent_type && TK_STRUCTURE == parent_type->get_kind())
        {
            structure_descriptor->base_type(parent_type);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid baseType found into 'structDcl'. Name: " << baseType);
            return XMLP_ret::XML_ERROR;
        }
    }

    traits<DynamicTypeBuilder>::ref_type type_builder = DynamicTypeBuilderFactory::get_instance()->create_type(
        structure_descriptor);

    const char* element_name = nullptr;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, MEMBER) == 0)
        {
            if (!parseXMLMemberDynamicType(p_element, type_builder, mId++))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'structDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, type_builder->build());

    return ret;
}

XMLP_ret XMLParser::parseXMLUnionDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="caseDcl">
            <xs:sequence>
                <xs:choice minOccurs="1" maxOccurs="unbounded">
                    <xs:element name="caseValue" type="string" minOccurs="1" maxOccurs="unbounded"/>
                </xs:choice>
                <xs:element name="member" type="memberDcl" minOccurs="1" maxOccurs="1"/>
            </xs:sequence>
        </xs:complexType>

        <xs:complexType name="unionDcl">
            <xs:sequence>
                <xs:element name="discriminator" type="string" minOccurs="1"/>
                <xs:sequence maxOccurs="unbounded">
                    <xs:element name="case" type="caseDcl" minOccurs="1"/>
                </xs:sequence>
            </xs:sequence>
            <xs:attribute name="name" type="identifierName" use="required"/>
        </xs:complexType>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* name = p_root->Attribute(NAME);
    tinyxml2::XMLElement* p_element = p_root->FirstChildElement(DISCRIMINATOR);
    if (p_element != nullptr)
    {
        const char* disc = p_element->Attribute(TYPE);
        traits<DynamicType>::ref_type disc_type = getDiscriminatorTypeBuilder(disc);
        if (!disc_type)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Error parsing union discriminator: Only primitive types allowed (found type " << disc << ").");
            ret = XMLP_ret::XML_ERROR;
        }
        else
        {
            traits<TypeDescriptorImpl>::ref_type union_descriptor {std::make_shared<TypeDescriptorImpl>(TK_UNION,
                                                                           name)};
            union_descriptor->discriminator_type(disc_type);
            traits<DynamicTypeBuilder>::ref_type type_builder {DynamicTypeBuilderFactory::get_instance()->
                                                                       create_type(union_descriptor)};

            MemberId mId{1};
            for (p_element = p_root->FirstChildElement(CASE);
                    p_element != nullptr; p_element = p_element->NextSiblingElement(CASE))
            {
                std::string valuesStr = "";
                for (tinyxml2::XMLElement* caseValue = p_element->FirstChildElement(CASE_DISCRIMINATOR);
                        caseValue != nullptr; caseValue = caseValue->NextSiblingElement(CASE_DISCRIMINATOR))
                {
                    const char* values = caseValue->Attribute(VALUE);
                    if (values == nullptr)
                    {
                        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing union case value: Not found.");
                        return XMLP_ret::XML_ERROR;
                    }

                    if (valuesStr.empty())
                    {
                        valuesStr = values;
                    }
                    else
                    {
                        valuesStr += std::string(",") + values;
                    }
                }

                tinyxml2::XMLElement* caseElement = p_element->FirstChildElement();
                while (caseElement != nullptr && strncmp(caseElement->Value(), CASE_DISCRIMINATOR, 10) == 0)
                {
                    caseElement = caseElement->NextSiblingElement();
                }
                if (caseElement != nullptr)
                {
                    if (!parseXMLMemberDynamicType(caseElement, type_builder, mId++, valuesStr))
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing union case member: Not found.");
                    return XMLP_ret::XML_ERROR;
                }
            }

            XMLProfileManager::insertDynamicTypeByName(name, type_builder->build());
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing union discriminator: Not found.");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

static void dimensionsToArrayBounds(
        const std::string& dimensions,
        std::vector<uint32_t>& bounds)
{
    std::stringstream ss(dimensions);
    std::string item;

    bounds.clear();

    while (std::getline(ss, item, ','))
    {
        bounds.push_back(static_cast<uint32_t>(std::atoi(item.c_str())));
    }
}

static bool dimensionsToLabels(
        const std::string& labelStr,
        UnionCaseLabelSeq& labels)
{
    std::stringstream ss(labelStr);
    std::string item;
    bool def = false;

    labels.clear();
    while (std::getline(ss, item, ','))
    {
        if (item == DEFAULT)
        {
            def = true;
        }
        else
        {
            labels.push_back(static_cast<uint32_t>(std::atoi(item.c_str())));
        }
    }

    return def;
}

traits<DynamicType>::ref_type XMLParser:: parseXMLMemberDynamicType(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="memberDcl">
            <xs:attribute name="name" type="string" use="required"/>
            <xs:attribute name="type" type="string" use="required"/>
            <xs:attribute name="arrayDimensions" type="string" use="optional"/>
            <xs:attribute name="nonBasic" type="string" use="optional"/>
            <xs:attribute name="sequenceMaxLength" type="string" use="optional"/>
            <xs:attribute name="mapMaxLength" type="string" use="optional"/>
            <xs:sequence>
                <xs:element name="member" type="memberDcl" minOccurs="0"/>
            </xs:sequence>
        </xs:complexType>
     */
    if (p_root == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member: Node not found.");
        return {};
    }

    const char* memberType = p_root->Attribute(TYPE);
    const char* memberName = p_root->Attribute(NAME);
    bool isArray = false;

    if (memberName == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member name: Not found.");
        return {};
    }

    if (memberType == nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member type: Not found.");
        return {};
    }

    const char* memberArray = p_root->Attribute(ARRAY_DIMENSIONS);
    if (memberArray != nullptr)
    {
        isArray = true;
    }

    if (strcmp(memberType, NON_BASIC_TYPE) == 0)
    {
        const char* memberNonBasicTypeName = p_root->Attribute(NON_BASIC_TYPE_NAME);
        if (memberNonBasicTypeName != nullptr)
        {
            memberType = memberNonBasicTypeName;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member type: Not found.");
            return {};
        }
    }

    traits<DynamicType>::ref_type member;
    traits<DynamicTypeBuilderFactory>::ref_type factory {DynamicTypeBuilderFactory::get_instance()};

    const char* memberSequence = p_root->Attribute(SEQ_MAXLENGTH);
    if (memberSequence != nullptr)
    {
        /*
            In sequences allowed formats are (complex format includes the basic):
            sequence<sequence<long,2>,2>
            <sequence name="my_sequence" length="2">
                <sequence name="inner_sequence" type="long" length="2"/>
            </sequence>
            In this example, inner sequence's name is ignored and can be omited.
         */
        traits<DynamicType>::ref_type content_type {getDiscriminatorTypeBuilder(memberType)};
        if (!content_type)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing sequence element type: Cannot be recognized: " << memberType);
            return {};
        }

        const char* lengthStr = p_root->Attribute(SEQ_MAXLENGTH);
        uint32_t length = MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            try
            {
                length = static_cast<uint32_t>(std::stoi(lengthStr));
            }
            catch (const std::exception&)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing member sequence length in line " << p_root->GetLineNum());
                return {};
            }
        }

        if (!isArray)
        {
            member = factory->create_sequence_type(content_type, length)->build();
        }
        else
        {
            traits<DynamicTypeBuilder>::ref_type inner_builder {factory->create_sequence_type(content_type, length)};
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(inner_builder->build(), bounds)->build();
        }
    }
    else if (p_root->Attribute(MAP_MAXLENGTH) != nullptr)
    {
        /*
            In maps allowed formats are (complex format includes the basic):
            map<map<long, long, 6>, map<long, map<long, short>,2>
            <map name="my_map" length="2">
                <key_type>
                    <map name="inner_key_map" key_type="long" value_type="long" length="6"/>
                </key_type>
                </value_type>
                    <map name="inner_value_map" key_type="long" length="2">
                        </value_type>
                            <map name="inner_value_value_map" key_type="long" value_type="short"/>
                        </value_type>
                    </map>
                </value_type>
            </map>
            In this example, inner maps names are ignored and can be omited.
         */
        // Parse key

        //const char* keyType = p_root->Attribute(KEY);
        traits<DynamicType>::ref_type key_type;
        const char* memberMapKeyType = p_root->Attribute(MAP_KEY_TYPE);
        if (memberMapKeyType != nullptr)
        {
            key_type = getDiscriminatorTypeBuilder(memberMapKeyType);
            if (!key_type)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing map's key element type: Cannot be recognized.");
                return {};
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing key_type element: Not found.");
            return {};
        }

        // Parse value
        traits<DynamicType>::ref_type value_type;
        if (memberType != nullptr)
        {
            value_type = getDiscriminatorTypeBuilder(memberType);
            if (!value_type)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing map's value element type: Cannot be recognized.");
                return {};
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing value_value element: Not found.");
            return {};
        }

        const char* lengthStr = p_root->Attribute(MAP_MAXLENGTH);
        uint32_t length = MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            try
            {
                length = static_cast<uint32_t>(std::stoi(lengthStr));
            }
            catch (const std::exception&)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Error parsing map member sequence length in line " << p_root->GetLineNum());
                return {};
            }
        }

        if (!isArray)
        {
            member = factory->create_map_type(key_type, value_type, length)->build();
        }
        else
        {
            traits<DynamicTypeBuilder>::ref_type inner_builder {factory->create_map_type(key_type, value_type,
                                                                        length)};
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                inner_builder->build(),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, BOOLEAN, 8) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_BOOLEAN);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_BOOLEAN),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, CHAR, 5) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_CHAR8);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_CHAR8),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, WCHAR, 6) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_CHAR16);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_CHAR16),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, TBYTE, 6) == 0
            || strncmp(memberType, OCTET, 6) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_BYTE);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_BYTE),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, UINT8, 6) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_UINT8);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_UINT8),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, INT8, 5) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_INT8);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_INT8),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, SHORT, 6) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_INT16);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_INT16),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, LONG, 5) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_INT32);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_INT32),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, ULONG, 13) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_UINT32);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_UINT32),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, USHORT, 14) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_UINT16);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_UINT16),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, LONGLONG, 9) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_INT64);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_INT64),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, ULONGLONG, 17) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_UINT64);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_UINT64),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, FLOAT, 6) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_FLOAT32);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_FLOAT32),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, DOUBLE, 7) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_FLOAT64);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_FLOAT64),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, LONGDOUBLE, 11) == 0)
    {
        if (!isArray)
        {
            member = factory->get_primitive_type(TK_FLOAT128);
        }
        else
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                factory->get_primitive_type(TK_FLOAT128),
                bounds)->build();
        }
    }
    else if (strncmp(memberType, STRING, 7) == 0)
    {
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        uint32_t bound = LENGTH_UNLIMITED;

        if (nullptr != boundStr)
        {
            bound = static_cast<uint32_t>(std::atoi(boundStr));
        }

        traits<DynamicTypeBuilder>::ref_type string_builder = factory->create_string_type(bound);

        if (!isArray)
        {
            member = string_builder->build();
        }
        else
        {
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            member = factory->create_array_type(
                string_builder->build(),
                boundsArray)->build();
        }
    }
    else if (strncmp(memberType, WSTRING, 8) == 0)
    {
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        uint32_t bound = LENGTH_UNLIMITED;

        if (nullptr != boundStr)
        {
            bound = static_cast<uint32_t>(std::atoi(boundStr));
        }

        traits<DynamicTypeBuilder>::ref_type wstring_builder = factory->create_wstring_type(bound);

        if (!isArray)
        {
            member = wstring_builder->build();
        }
        else
        {
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            member = factory->create_array_type(
                wstring_builder->build(),
                boundsArray)->build();
        }
    }
    else // Complex type?
    {
        traits<DynamicType>::ref_type type;
        XMLProfileManager::getDynamicTypeByName(type, memberType);
        if (!isArray)
        {
            member = type;
        }
        else if (type)
        {
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            member = factory->create_array_type(
                type,
                bounds)->build();
        }
    }

    if (!member)
    {
        if (!isArray)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Failed creating " << memberType << ": " << (memberName ? memberName : ""));
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Failed creating " << memberType << " array: " << (memberName ? memberName : ""));
        }
        return {};
    }

    /*TODO(richiware)
       const char* memberTopicKey = p_root->Attribute(KEY);

       if (memberTopicKey != nullptr)
       {
        if (strncmp(memberTopicKey, "true", 5) == 0)
        {
            // check if its primitive
            if (memberBuilder->is_primitive())
            {
                auto& fact = DynamicTypeBuilderFactory::get_instance();

                // copy if modification is required
                DynamicTypeBuilder* mutable_builder {factory.create_type_copy(*memberBuilder)};

                   AnnotationDescriptor ad;
                   ad.set_type(fact.create_annotation_primitive(ANNOTATION_KEY_ID));
                   ad.set_value("value", CONST_TRUE);
                   mutable_builder->apply_annotation(ad);

                return mutable_builder;
            }
        }
       }
     */

    return member;
}

traits<DynamicType>::ref_type XMLParser::parseXMLMemberDynamicType(
        tinyxml2::XMLElement* p_root,
        traits<fastdds::dds::DynamicTypeBuilder>::ref_type& builder,
        MemberId mId)
{
    return parseXMLMemberDynamicType(p_root, builder, mId, "");
}

traits<DynamicType>::ref_type XMLParser::parseXMLMemberDynamicType(
        tinyxml2::XMLElement* p_root,
        traits<fastdds::dds::DynamicTypeBuilder>::ref_type& builder,
        MemberId mId,
        const std::string& values)
{
    traits<DynamicType>::ref_type member {parseXMLMemberDynamicType(p_root)};

    if (!member)
    {
        return {};
    }

    /*TODO(richiware)
       const char* memberTopicKey = p_root->Attribute(KEY);

       if (memberTopicKey != nullptr && strncmp(memberTopicKey, "true", 5) == 0)
       {
       auto& fact = DynamicTypeBuilderFactory::get_instance();

       AnnotationDescriptor ad;
       ad.set_type(fact.create_annotation_primitive(ANNOTATION_KEY_ID));
       ad.set_value("value", CONST_TRUE);
       builder.apply_annotation(ad);
       }
     */

    const char* memberName = p_root->Attribute(NAME);

    traits<MemberDescriptorImpl>::ref_type md {std::make_shared<MemberDescriptorImpl>()};
    md->id(mId);
    md->name(memberName);
    md->type(member);

    if (memberName != nullptr && !values.empty())
    {
        UnionCaseLabelSeq labels;
        bool defaultLabel = dimensionsToLabels(values, labels);

        md->label(labels);
        md->is_default_label(defaultLabel);
    }

    builder->add_member(md);

    return member;
}

XMLP_ret XMLParser::parseXMLDynamicTypes(
        tinyxml2::XMLElement& types)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    ret  = parseDynamicTypes(&types);
    return ret;
}

XMLP_ret XMLParser::parseDynamicTypes(
        tinyxml2::XMLElement* p_root)
{
    return parseXMLTypes(p_root);
}

XMLP_ret XMLParser::loadXMLDynamicTypes(
        tinyxml2::XMLElement& xmlDoc)
{
    return parseXMLDynamicTypes(xmlDoc);
}
