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

#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLTree.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>

#include <tinyxml2.h>
#include <iostream>
#include <cstdlib>

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
            logError(XMLPARSER, "Error parsing type: Type " << type << " not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }

        if (ret != XMLP_ret::XML_OK)
        {
            logError(XMLPARSER, "Error parsing type " << type << ".");
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
                logError(XMLPARSER, "Invalid element found into 'types'. Name: " << name);
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
                logError(XMLPARSER, "Invalid element found into 'types'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLBitvalueDynamicType(
        tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype,
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
        logError(XMLPARSER, "Error parsing bitmask: Node not found.");
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
            logError(XMLPARSER, "Error parsing bit_value position: Invalid (must be an unsigned short).");
            return XMLP_ret::XML_ERROR;
        }
    }

    if (memberName == nullptr && p_dynamictype != nullptr)
    {
        logError(XMLPARSER, "Error parsing bit_value name: Not found.");
        return XMLP_ret::XML_ERROR;
    }

    p_dynamictype->add_empty_member(field_position, memberName);
    //p_dynamictype->apply_annotation_to_member(
    //    p_dynamictype->get_member_id_by_name(memberName), ANNOTATION_POSITION_ID, "value", position);
    ++field_position;

    return XMLP_ret::XML_OK;
}

static p_dynamictypebuilder_t getDiscriminatorTypeBuilder(
        const std::string& disc,
        uint32_t bound = 0);

static p_dynamictypebuilder_t getDiscriminatorTypeBuilder(
        const std::string& disc,
        uint32_t bound)
{
    /*
       mKind == TK_BOOLEAN || mKind == TK_BYTE || mKind == TK_INT16 || mKind == TK_INT32 ||
        mKind == TK_INT64 || mKind == TK_UINT16 || mKind == TK_UINT32 || mKind == TK_UINT64 ||
        mKind == TK_FLOAT32 || mKind == TK_FLOAT64 || mKind == TK_FLOAT128 || mKind == TK_CHAR8 ||
        mKind == TK_CHAR16 || mKind == TK_STRING8 || mKind == TK_STRING16 || mKind == TK_ENUM || mKind == TK_BITMASK
     */
    types::DynamicTypeBuilderFactory* factory = types::DynamicTypeBuilderFactory::get_instance();
    if (disc.compare(BOOLEAN) == 0)
    {
        return factory->create_bool_builder();
    }
    else if (disc.compare(TBYTE) == 0
            || disc.compare(OCTET) == 0
            || disc.compare(INT8) == 0
            || disc.compare(UINT8) == 0)
    {
        return factory->create_byte_builder();
    }
    else if (disc.compare(SHORT) == 0)
    {
        return factory->create_int16_builder();
    }
    else if (disc.compare(LONG) == 0)
    {
        return factory->create_int32_builder();
    }
    else if (disc.compare(LONGLONG) == 0)
    {
        return factory->create_int64_builder();
    }
    else if (disc.compare(USHORT) == 0)
    {
        return factory->create_uint16_builder();
    }
    else if (disc.compare(ULONG) == 0)
    {
        return factory->create_uint32_builder();
    }
    else if (disc.compare(ULONGLONG) == 0)
    {
        return factory->create_uint64_builder();
    }
    else if (disc.compare(FLOAT) == 0)
    {
        return factory->create_float32_builder();
    }
    else if (disc.compare(DOUBLE) == 0)
    {
        return factory->create_float64_builder();
    }
    else if (disc.compare(LONGDOUBLE) == 0)
    {
        return factory->create_float128_builder();
    }
    else if (disc.compare(CHAR) == 0)
    {
        return factory->create_char8_builder();
    }
    else if (disc.compare(WCHAR) == 0)
    {
        return factory->create_char16_builder();
    }
    else if (disc.compare(STRING) == 0)
    {
        return factory->create_string_builder(bound);
    }
    else if (disc.compare(WSTRING) == 0)
    {
        return factory->create_wstring_builder(bound);
    }

    return XMLProfileManager::getDynamicTypeByName(disc);
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
                logError(XMLPARSER, "Error parsing member type: Not found.");
                ret = XMLP_ret::XML_ERROR;
            }
        }

        p_dynamictypebuilder_t valueBuilder;
        if ((p_root->Attribute(ARRAY_DIMENSIONS) != nullptr) ||
                (p_root->Attribute(SEQ_MAXLENGTH) != nullptr) ||
                (p_root->Attribute(MAP_MAXLENGTH) != nullptr))
        {
            valueBuilder = parseXMLMemberDynamicType(p_root, nullptr, MEMBER_ID_INVALID);
        }
        else
        {
            uint32_t bound = 0;
            const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
            if (boundStr != nullptr)
            {
                bound = static_cast<uint32_t>(std::atoi(boundStr));
            }
            valueBuilder = getDiscriminatorTypeBuilder(type, bound);
        }

        if (valueBuilder != nullptr)
        {
            const char* name = p_root->Attribute(NAME);
            p_dynamictypebuilder_t typeBuilder =
                    types::DynamicTypeBuilderFactory::get_instance()->create_alias_builder(valueBuilder, name);
            XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
        }
        else
        {
            logError(XMLPARSER, "Error parsing alias type: Value not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Error parsing alias type: Type not defined.");
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
    p_dynamictypebuilder_t typeBuilder;
    uint32_t mId = 0;

    const char* name = p_root->Attribute(NAME);

    const char* baseType = p_root->Attribute(BASE_TYPE);
    if (baseType != nullptr)
    {
        p_dynamictypebuilder_t parentType = XMLProfileManager::getDynamicTypeByName(baseType);
        if (parentType != nullptr && parentType->get_kind() == types::TK_BITSET)
        {
            typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_child_struct_builder(parentType);
        }
        else
        {
            logError(XMLPARSER, "Invalid baseType found into 'bitsetDcl'. Name: " << baseType);
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_bitset_builder();
    }
    typeBuilder->set_name(name);

    const char* element_name = nullptr;
    uint16_t position = 0;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, BITFIELD) == 0)
        {
            p_dynamictypebuilder_t mType = parseXMLBitfieldDynamicType(p_element, typeBuilder, mId++, position);
            if (mType == nullptr)
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'bitsetDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
    return ret;
}

p_dynamictypebuilder_t XMLParser::parseXMLBitfieldDynamicType(
        tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype,
        types::MemberId mId,
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
        logError(XMLPARSER, "Error parsing bitfield: Node not found.");
        return nullptr;
    }

    const char* memberType = p_root->Attribute(TYPE);
    const char* memberName = p_root->Attribute(NAME);
    const char* bit_bound = p_root->Attribute(BIT_BOUND);

    if (bit_bound == nullptr && p_dynamictype != nullptr)
    {
        logError(XMLPARSER, "Error parsing bitfield bit_bound: Not found.");
        return nullptr;
    }

    if (memberName == nullptr)
    {
        memberName = "";
    }

    types::DynamicTypeBuilder* memberBuilder = nullptr;
    types::DynamicTypeBuilderFactory* factory = types::DynamicTypeBuilderFactory::get_instance();

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
                logError(XMLPARSER, "Failed creating bitfield, size too big: " << bit_bound);
                return nullptr;
            }
        }
        catch (...)
        {
            logError(XMLPARSER, "Failed creating bitfield, invalid bit_bound (must be an unsigned short): "
                    << bit_bound);
            return nullptr;
        }
    }

    if (strncmp(memberType, BOOLEAN, 8) == 0)
    {
        memberBuilder = factory->create_bool_builder();
    }
    else if (strncmp(memberType, CHAR, 5) == 0)
    {
        memberBuilder = factory->create_char8_builder();
    }
    else if (strncmp(memberType, WCHAR, 6) == 0)
    {
        memberBuilder = factory->create_char16_builder();
    }
    else if (strncmp(memberType, TBYTE, 6) == 0
            || strncmp(memberType, OCTET, 6) == 0
            || strncmp(memberType, UINT8, 6) == 0
            || strncmp(memberType, INT8, 5) == 0)
    {
        memberBuilder = factory->create_byte_builder();
    }
    else if (strncmp(memberType, SHORT, 6) == 0)
    {
        memberBuilder = factory->create_int16_builder();
    }
    else if (strncmp(memberType, LONG, 5) == 0)
    {
        memberBuilder = factory->create_int32_builder();
    }
    else if (strncmp(memberType, ULONG, 13) == 0)
    {
        memberBuilder = factory->create_uint32_builder();
    }
    else if (strncmp(memberType, USHORT, 14) == 0)
    {
        memberBuilder = factory->create_uint16_builder();
    }
    else if (strncmp(memberType, LONGLONG, 9) == 0)
    {
        memberBuilder = factory->create_int64_builder();
    }
    else if (strncmp(memberType, ULONGLONG, 17) == 0)
    {
        memberBuilder = factory->create_uint64_builder();
    }
    else // Unsupported type?
    {
        logError(XMLPARSER, "Failed creating bitfield " << memberName << ": Type " << memberType << " unsupported.");
    }


    if (memberBuilder == nullptr)
    {
        logError(XMLPARSER, "Failed creating " << memberType << ": " << memberName);
    }

    if (p_dynamictype != nullptr)
    {
        p_dynamictype->add_member(mId, memberName, memberBuilder);
        if (!std::string(memberName).empty())
        {
            p_dynamictype->apply_annotation_to_member(mId, types::ANNOTATION_BIT_BOUND_ID, "value", bit_bound);
            //position += static_cast<uint16_t>(mId);
            p_dynamictype->apply_annotation_to_member(mId, types::ANNOTATION_POSITION_ID, "value",
                    std::to_string(position));
        }
        position += static_cast<uint16_t>(atoi(bit_bound));
    }

    return memberBuilder;
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
        bit_bound = static_cast<uint16_t>(std::atoi(anno_bit_bound));
    }

    const char* name = p_root->Attribute(NAME);
    p_dynamictypebuilder_t typeBuilder =
            types::DynamicTypeBuilderFactory::get_instance()->create_bitmask_builder(bit_bound);
    typeBuilder->set_name(name);
    uint16_t position = 0;

    const char* element_name = nullptr;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, BIT_VALUE) == 0)
        {
            if (parseXMLBitvalueDynamicType(p_element, typeBuilder, position) != XMLP_ret::XML_OK)
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'bitmaskDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
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
    p_dynamictypebuilder_t typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_enum_builder();
    uint32_t currValue = 0;
    for (tinyxml2::XMLElement* literal = p_root->FirstChildElement(ENUMERATOR);
            literal != nullptr; literal = literal->NextSiblingElement(ENUMERATOR))
    {
        const char* name = literal->Attribute(NAME);
        if (name == nullptr)
        {
            logError(XMLPARSER, "Error parsing enum type: Literals must have name.");
            return XMLP_ret::XML_ERROR;
        }

        const char* value = literal->Attribute(VALUE);
        if (value != nullptr)
        {
            currValue = static_cast<uint32_t>(std::atoi(value));
        }
        typeBuilder->add_empty_member(currValue++, name);
    }

    XMLProfileManager::insertDynamicTypeByName(enumName, typeBuilder);
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
    p_dynamictypebuilder_t typeBuilder; // = types::DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
    //typeBuilder->set_name(name);
    uint32_t mId = 0;

    const char* baseType = p_root->Attribute(BASE_TYPE);
    if (baseType != nullptr)
    {
        p_dynamictypebuilder_t parentType = XMLProfileManager::getDynamicTypeByName(baseType);
        if (parentType != nullptr && parentType->get_kind() == types::TK_STRUCTURE)
        {
            typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_child_struct_builder(parentType);
        }
        else
        {
            logError(XMLPARSER, "Invalid baseType found into 'structDcl'. Name: " << baseType);
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_struct_builder();
    }
    typeBuilder->set_name(name);

    const char* element_name = nullptr;
    for (tinyxml2::XMLElement* p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, MEMBER) == 0)
        {
            p_dynamictypebuilder_t mType = parseXMLMemberDynamicType(p_element, typeBuilder, mId++);
            if (mType == nullptr)
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'structDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);

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
        p_dynamictypebuilder_t discriminator = getDiscriminatorTypeBuilder(disc);
        if (discriminator == nullptr)
        {
            logError(XMLPARSER,
                    "Error parsing union discriminator: Only primitive types allowed (found type " << disc << ").");
            ret = XMLP_ret::XML_ERROR;
        }
        else
        {
            p_dynamictypebuilder_t typeBuilder = types::DynamicTypeBuilderFactory::get_instance()->create_union_builder(
                discriminator);
            typeBuilder->set_name(name);

            uint32_t mId = 0;
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
                        logError(XMLPARSER, "Error parsing union case value: Not found.");
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
                    p_dynamictypebuilder_t mType = parseXMLMemberDynamicType
                                (caseElement, typeBuilder, mId++, valuesStr);
                    if (mType == nullptr)
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                }
                else
                {
                    logError(XMLPARSER, "Error parsing union case member: Not found.");
                    return XMLP_ret::XML_ERROR;
                }
            }

            XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
        }
    }
    else
    {
        logError(XMLPARSER, "Error parsing union discriminator: Not found.");
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
        std::vector<uint64_t>& labels)
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
            labels.push_back(static_cast<uint64_t>(std::atoi(item.c_str())));
        }
    }

    return def;
}

p_dynamictypebuilder_t XMLParser::parseXMLMemberDynamicType(
        tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype,
        types::MemberId mId)
{
    return parseXMLMemberDynamicType(p_root, p_dynamictype, mId, "");
}

p_dynamictypebuilder_t XMLParser::parseXMLMemberDynamicType(
        tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype,
        types::MemberId mId,
        const std::string& values)
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
        logError(XMLPARSER, "Error parsing member: Node not found.");
        return nullptr;
    }

    const char* memberType = p_root->Attribute(TYPE);
    const char* memberName = p_root->Attribute(NAME);
    bool isArray = false;

    if (memberName == nullptr && p_dynamictype != nullptr)
    {
        logError(XMLPARSER, "Error parsing member name: Not found.");
        return nullptr;
    }

    if (memberType == nullptr)
    {
        logError(XMLPARSER, "Error parsing member type: Not found.");
        return nullptr;
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
            logError(XMLPARSER, "Error parsing member type: Not found.");
            return nullptr;
        }
    }

    types::DynamicTypeBuilder* memberBuilder = nullptr;
    types::DynamicTypeBuilderFactory* factory = types::DynamicTypeBuilderFactory::get_instance();

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
        p_dynamictypebuilder_t contentType = getDiscriminatorTypeBuilder(memberType);
        if (contentType == nullptr)
        {
            logError(XMLPARSER, "Error parsing sequence element type: Cannot be recognized: " << memberType);
            return nullptr;
        }

        const char* lengthStr = p_root->Attribute(SEQ_MAXLENGTH);
        uint32_t length = types::MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            length = static_cast<uint32_t>(std::stoi(lengthStr));
        }

        if (!isArray)
        {
            memberBuilder = factory->create_sequence_builder(contentType, length);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_sequence_builder(contentType, length);
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
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
        p_dynamictypebuilder_t keyTypeBuilder = nullptr;
        const char* memberMapKeyType = p_root->Attribute(MAP_KEY_TYPE);
        if (memberMapKeyType != nullptr)
        {
            keyTypeBuilder = getDiscriminatorTypeBuilder(memberMapKeyType);
            if (keyTypeBuilder == nullptr)
            {
                logError(XMLPARSER, "Error parsing map's key element type: Cannot be recognized.");
                return nullptr;
            }
        }
        else
        {
            logError(XMLPARSER, "Error parsing key_type element: Not found.");
            return nullptr;
        }

        // Parse value
        p_dynamictypebuilder_t valueTypeBuilder;
        if (memberType != nullptr)
        {
            valueTypeBuilder = getDiscriminatorTypeBuilder(memberType);
            if (valueTypeBuilder == nullptr)
            {
                logError(XMLPARSER, "Error parsing map's value element type: Cannot be recognized.");
                return nullptr;
            }
        }
        else
        {
            logError(XMLPARSER, "Error parsing value_value element: Not found.");
            return nullptr;
        }

        const char* lengthStr = p_root->Attribute(MAP_MAXLENGTH);
        uint32_t length = types::MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            length = static_cast<uint32_t>(std::stoi(lengthStr));
        }

        if (!isArray)
        {
            memberBuilder = factory->create_map_builder(keyTypeBuilder, valueTypeBuilder, length);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder =
                    factory->create_map_builder(keyTypeBuilder, valueTypeBuilder, length);
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, BOOLEAN, 8) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_bool_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_bool_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, CHAR, 5) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_char8_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_char8_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, WCHAR, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_char16_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_char16_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, TBYTE, 6) == 0
            || strncmp(memberType, OCTET, 6) == 0
            || strncmp(memberType, UINT8, 6) == 0
            || strncmp(memberType, INT8, 5) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_byte_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_byte_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, SHORT, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_int16_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_int16_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONG, 5) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_int32_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_int32_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, ULONG, 13) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_uint32_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_uint32_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, USHORT, 14) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_uint16_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_uint16_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONGLONG, 9) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_int64_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_int64_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, ULONGLONG, 17) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_uint64_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_uint64_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, FLOAT, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_float32_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_float32_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, DOUBLE, 7) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_float64_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_float64_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONGDOUBLE, 11) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->create_float128_builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_float128_builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, STRING, 7) == 0)
    {
        uint32_t bound = 0;
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        if (boundStr != nullptr)
        {
            bound = static_cast<uint32_t>(std::atoi(boundStr));
        }
        if (!isArray)
        {
            memberBuilder = factory->create_string_builder(bound);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_string_builder(bound);
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            memberBuilder = factory->create_array_builder(innerBuilder, boundsArray);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, WSTRING, 8) == 0)
    {
        uint32_t bound = 0;
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        if (boundStr != nullptr)
        {
            bound = static_cast<uint32_t>(std::atoi(boundStr));
        }
        if (!isArray)
        {
            memberBuilder = factory->create_wstring_builder(bound);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->create_wstring_builder(bound);
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            memberBuilder = factory->create_array_builder(innerBuilder, boundsArray);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else // Complex type?
    {
        p_dynamictypebuilder_t typePtr = XMLProfileManager::getDynamicTypeByName(memberType);
        if (!isArray)
        {
            memberBuilder = typePtr;
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = typePtr;
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->create_array_builder(innerBuilder, bounds);
            // Don't delete innerBuilder, it will be freed on destructors.
        }
    }


    if (memberBuilder == nullptr)
    {
        if (!isArray)
        {
            logError(XMLPARSER, "Failed creating " << memberType << ": " << memberName);
        }
        else
        {
            logError(XMLPARSER, "Failed creating " << memberType << " array: " << memberName);
        }
    }

    const char* memberTopicKey = p_root->Attribute(KEY);
    if (memberTopicKey != nullptr)
    {
        if (strncmp(memberTopicKey, "true", 5) == 0)
        {
            memberBuilder->apply_annotation(types::ANNOTATION_KEY_ID, "value", "true");
            if (p_dynamictype != nullptr)
            {
                p_dynamictype->apply_annotation(types::ANNOTATION_KEY_ID, "value", "true");
            }
        }
    }

    if (p_dynamictype != nullptr)
    {
        if (!values.empty())
        {
            std::vector<uint64_t> labels;
            bool defaultLabel = dimensionsToLabels(values, labels);
            p_dynamictype->add_member(mId, memberName, memberBuilder,
                    "", labels, defaultLabel);
        }
        else
        {
            p_dynamictype->add_member(mId, memberName, memberBuilder);
        }
    }
    //factory->DeleteBuilder(memberBuilder);

    return memberBuilder;
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
