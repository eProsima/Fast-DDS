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

#include <security/accesscontrol/GovernanceParser.h>
#include <fastdds/dds/log/Log.hpp>

#include <cstring>
#include <cassert>

#if TIXML2_MAJOR_VERSION >= 6
#define PRINTLINE(node) node->GetLineNum()
#define PRINTLINEPLUSONE(node) node->GetLineNum() + 1
#else
#define PRINTLINE(node) ""
#define PRINTLINEPLUSONE(node) ""
#endif // if TIXML2_MAJOR_VERSION >= 6

static const char* Root_str = "dds";
static const char* DomainAccessRules_str = "domain_access_rules";
static const char* DomainRule_str = "domain_rule";
static const char* Domains_str = "domains";
static const char* AllowUnauthenticatedParticipants_str = "allow_unauthenticated_participants";
static const char* EnableJoinAccessControl_str = "enable_join_access_control";
static const char* DiscoveryProtectionKind_str = "discovery_protection_kind";
static const char* LivelinessProtectionKind_str = "liveliness_protection_kind";
static const char* RtpsProtectionKind_str = "rtps_protection_kind";
static const char* TopicAccessRules_str = "topic_access_rules";
static const char* TopicRule_str = "topic_rule";
static const char* TopicExpression_str = "topic_expression";
static const char* EnableDiscoveryProtection_str = "enable_discovery_protection";
static const char* EnableLivelinessProtection_str = "enable_liveliness_protection";
static const char* EnableReadAccessControl_str = "enable_read_access_control";
static const char* EnableWriteAccessControl_str = "enable_write_access_control";
static const char* MetadataProtectionKind_str = "metadata_protection_kind";
static const char* DataProtectionKind_str = "data_protection_kind";

static const char* ProtectionKindNone_str = "NONE";
static const char* ProtectionKindSign_str = "SIGN";
static const char* ProtectionKindEncrypt_str = "ENCRYPT";
static const char* ProtectionKindSignAuth_str = "SIGN_WITH_ORIGIN_AUTHENTICATION";
static const char* ProtectionKindEncryptAuth_str = "ENCRYPT_WITH_ORIGIN_AUTHENTICATION";

using namespace eprosima::fastdds::rtps::security;

void GovernanceParser::swap(
        DomainAccessRules& rules)
{
    rules = std::move(access_rules_);
}

bool GovernanceParser::parse_stream(
        const char* stream,
        size_t stream_length)
{
    assert(stream);

    bool returned_value = false;
    tinyxml2::XMLDocument document;

    if (tinyxml2::XMLError::XML_SUCCESS == document.Parse(stream, stream_length))
    {
        tinyxml2::XMLElement* root = document.RootElement();

        if (root != nullptr)
        {
            if (strcmp(root->Name(), Root_str) == 0)
            {
                returned_value = parse_domain_access_rules_node(root);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Malformed Governance root. Line " << PRINTLINE(root));
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Not found root node in Governance XML.");
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error loading Governance XML");
    }

    return returned_value;
}

bool GovernanceParser::parse_domain_access_rules_node(
        tinyxml2::XMLElement* root)
{
    assert(root);

    bool returned_value = false;
    tinyxml2::XMLElement* node = root->FirstChildElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), DomainAccessRules_str) == 0)
        {
            if (parse_domain_access_rules(node))
            {
                if (node->NextSibling() == nullptr)
                {
                    returned_value = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Only permitted one " << DomainAccessRules_str << " tag. Line "
                                                                        << PRINTLINE(node->NextSibling()));
                }
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid tag. Expected " << DomainAccessRules_str << " tag. Line " << PRINTLINE(
                        node));
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << DomainAccessRules_str << " tag after root. Line " << PRINTLINEPLUSONE(
                    root));
    }

    return returned_value;
}

bool GovernanceParser::parse_domain_access_rules(
        tinyxml2::XMLElement* root)
{
    assert(root);

    bool returned_value = false;
    tinyxml2::XMLElement* node = root->FirstChildElement();

    if (node != nullptr)
    {
        returned_value = true;

        do
        {
            if (strcmp(node->Name(), DomainRule_str) == 0)
            {
                DomainRule domain_rule;

                if ((returned_value = parse_domain_rule(node, domain_rule)) == true)
                {
                    access_rules_.rules.push_back(std::move(domain_rule));
                }
            }
            else
            {
                returned_value = false;
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << DomainRule_str << " tag. Line " << PRINTLINE(node));
            }
        }
        while (returned_value && (node = node->NextSiblingElement()) != nullptr);
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Minimum one " << DomainRule_str << " tag. Line " << PRINTLINEPLUSONE(root));
    }

    return returned_value;
}

bool GovernanceParser::parse_domain_rule(
        tinyxml2::XMLElement* root,
        DomainRule& rule)
{
    assert(root);

    tinyxml2::XMLElement* node = root->FirstChildElement();
    tinyxml2::XMLElement* old_node = nullptr;
    (void)old_node;

    if (node != nullptr)
    {
        if (strcmp(node->Name(), Domains_str) == 0)
        {
            if (!parse_domain_id_set(node, rule.domains))
            {
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << Domains_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << Domains_str << " tag. Line " << PRINTLINEPLUSONE(root));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), AllowUnauthenticatedParticipants_str) == 0)
        {
            if (node->QueryBoolText(&rule.allow_unauthenticated_participants) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << AllowUnauthenticatedParticipants_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << AllowUnauthenticatedParticipants_str << " tag. Line " << PRINTLINE(
                        node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << AllowUnauthenticatedParticipants_str << " tag. Line " << PRINTLINEPLUSONE(
                    old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), EnableJoinAccessControl_str) == 0)
        {
            if (node->QueryBoolText(&rule.enable_join_access_control) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << EnableJoinAccessControl_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << EnableJoinAccessControl_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << EnableJoinAccessControl_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), DiscoveryProtectionKind_str) == 0)
        {
            const char* text = node->GetText();

            if (text != nullptr)
            {
                if (strcmp(text, ProtectionKindNone_str) == 0)
                {
                    rule.discovery_protection_kind = ProtectionKind::NONE;
                }
                else if (strcmp(text, ProtectionKindSign_str) == 0)
                {
                    rule.discovery_protection_kind = ProtectionKind::SIGN;
                }
                else if (strcmp(text, ProtectionKindEncrypt_str) == 0)
                {
                    rule.discovery_protection_kind = ProtectionKind::ENCRYPT;
                }
                else if (strcmp(text, ProtectionKindSignAuth_str) == 0)
                {
                    rule.discovery_protection_kind = ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION;
                }
                else if (strcmp(text, ProtectionKindEncryptAuth_str) == 0)
                {
                    rule.discovery_protection_kind = ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid text in " << DiscoveryProtectionKind_str << " tag. Line " << PRINTLINE(
                                node));
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected text in " << DiscoveryProtectionKind_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << DiscoveryProtectionKind_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << DiscoveryProtectionKind_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), LivelinessProtectionKind_str) == 0)
        {
            const char* text = node->GetText();

            if (text != nullptr)
            {
                if (strcmp(text, ProtectionKindNone_str) == 0)
                {
                    rule.liveliness_protection_kind = ProtectionKind::NONE;
                }
                else if (strcmp(text, ProtectionKindSign_str) == 0)
                {
                    rule.liveliness_protection_kind = ProtectionKind::SIGN;
                }
                else if (strcmp(text, ProtectionKindEncrypt_str) == 0)
                {
                    rule.liveliness_protection_kind = ProtectionKind::ENCRYPT;
                }
                else if (strcmp(text, ProtectionKindSignAuth_str) == 0)
                {
                    rule.liveliness_protection_kind = ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION;
                }
                else if (strcmp(text, ProtectionKindEncryptAuth_str) == 0)
                {
                    rule.liveliness_protection_kind = ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid text in " << LivelinessProtectionKind_str << " tag. Line " << PRINTLINE(
                                node));
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected text in " << LivelinessProtectionKind_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << LivelinessProtectionKind_str << " tag. Line " << PRINTLINE(
                        node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << LivelinessProtectionKind_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), RtpsProtectionKind_str) == 0)
        {
            const char* text = node->GetText();

            if (text != nullptr)
            {
                if (strcmp(text, ProtectionKindNone_str) == 0)
                {
                    rule.rtps_protection_kind = ProtectionKind::NONE;
                }
                else if (strcmp(text, ProtectionKindSign_str) == 0)
                {
                    rule.rtps_protection_kind = ProtectionKind::SIGN;
                }
                else if (strcmp(text, ProtectionKindEncrypt_str) == 0)
                {
                    rule.rtps_protection_kind = ProtectionKind::ENCRYPT;
                }
                else if (strcmp(text, ProtectionKindSignAuth_str) == 0)
                {
                    rule.rtps_protection_kind = ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION;
                }
                else if (strcmp(text, ProtectionKindEncryptAuth_str) == 0)
                {
                    rule.rtps_protection_kind = ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid text in " << RtpsProtectionKind_str << " tag. Line " << PRINTLINE(
                                node));
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected text in " << RtpsProtectionKind_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << RtpsProtectionKind_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << RtpsProtectionKind_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), TopicAccessRules_str) == 0)
        {
            if (!parse_topic_access_rules(node, rule.topic_rules))
            {
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << EnableJoinAccessControl_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << EnableJoinAccessControl_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected other tag. Line " << PRINTLINE(node));
        return false;
    }

    return true;
}

#include <iostream>
bool GovernanceParser::parse_topic_access_rules(
        tinyxml2::XMLElement* root,
        std::vector<TopicRule>& rules)
{
    assert(root);

    bool returned_value = false;
    tinyxml2::XMLElement* node = root->FirstChildElement();

    if (node != nullptr)
    {
        returned_value = true;

        do
        {
            if (strcmp(node->Name(), TopicRule_str) == 0)
            {
                TopicRule topic_rule;

                if ((returned_value = parse_topic_rule(node, topic_rule)) == true)
                {
                    rules.push_back(std::move(topic_rule));
                }
            }
            else
            {
                returned_value = false;
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << TopicRule_str << " tag. Line " << PRINTLINE(node));
            }
        }
        while (returned_value && (node = node->NextSiblingElement()) != nullptr);
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Minimum one " << TopicRule_str << " tag. Line " << PRINTLINEPLUSONE(root));
    }

    return returned_value;
}

bool GovernanceParser::parse_topic_rule(
        tinyxml2::XMLElement* root,
        TopicRule& rule)
{
    assert(root);

    tinyxml2::XMLElement* node = root->FirstChildElement();
    tinyxml2::XMLElement* old_node = nullptr;
    (void)old_node;

    if (node != nullptr)
    {
        if (strcmp(node->Name(), TopicExpression_str) == 0)
        {
            rule.topic_expression = node->GetText();
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << TopicExpression_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << TopicExpression_str << " tag. Line " << PRINTLINEPLUSONE(root));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), EnableDiscoveryProtection_str) == 0)
        {
            if (node->QueryBoolText(&rule.enable_discovery_protection) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << EnableDiscoveryProtection_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << EnableDiscoveryProtection_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << EnableDiscoveryProtection_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), EnableLivelinessProtection_str) == 0)
        {
            if (node->QueryBoolText(&rule.enable_liveliness_protection) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << EnableLivelinessProtection_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << EnableLivelinessProtection_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << EnableLivelinessProtection_str << " tag. Line " << PRINTLINEPLUSONE(
                    old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), EnableReadAccessControl_str) == 0)
        {
            if (node->QueryBoolText(&rule.enable_read_access_control) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << EnableReadAccessControl_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << EnableReadAccessControl_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << EnableReadAccessControl_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), EnableWriteAccessControl_str) == 0)
        {
            if (node->QueryBoolText(&rule.enable_write_access_control) != tinyxml2::XMLError::XML_SUCCESS)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected boolean value in " << EnableWriteAccessControl_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << EnableWriteAccessControl_str << " tag. Line " << PRINTLINE(
                        node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << EnableWriteAccessControl_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), MetadataProtectionKind_str) == 0)
        {
            const char* text = node->GetText();

            if (text != nullptr)
            {
                if (strcmp(text, ProtectionKindNone_str) == 0)
                {
                    rule.metadata_protection_kind = ProtectionKind::NONE;
                }
                else if (strcmp(text, ProtectionKindSign_str) == 0)
                {
                    rule.metadata_protection_kind = ProtectionKind::SIGN;
                }
                else if (strcmp(text, ProtectionKindEncrypt_str) == 0)
                {
                    rule.metadata_protection_kind = ProtectionKind::ENCRYPT;
                }
                else if (strcmp(text, ProtectionKindSignAuth_str) == 0)
                {
                    rule.metadata_protection_kind = ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION;
                }
                else if (strcmp(text, ProtectionKindEncryptAuth_str) == 0)
                {
                    rule.metadata_protection_kind = ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid text in " << MetadataProtectionKind_str << " tag. Line " << PRINTLINE(
                                node));
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected text in " << MetadataProtectionKind_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Expected " << MetadataProtectionKind_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << MetadataProtectionKind_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    old_node = node;
    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        if (strcmp(node->Name(), DataProtectionKind_str) == 0)
        {
            const char* text = node->GetText();

            if (text != nullptr)
            {
                if (strcmp(text, ProtectionKindNone_str) == 0)
                {
                    rule.data_protection_kind = ProtectionKind::NONE;
                }
                else if (strcmp(text, ProtectionKindSign_str) == 0)
                {
                    rule.data_protection_kind = ProtectionKind::SIGN;
                }
                else if (strcmp(text, ProtectionKindEncrypt_str) == 0)
                {
                    rule.data_protection_kind = ProtectionKind::ENCRYPT;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid text in " << DataProtectionKind_str << " tag. Line " << PRINTLINE(
                                node));
                    return false;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected text in " << DataProtectionKind_str << " tag. Line " << PRINTLINE(
                            node));
                return false;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected " << DataProtectionKind_str << " tag. Line " << PRINTLINE(node));
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Expected " << DataProtectionKind_str << " tag. Line " << PRINTLINEPLUSONE(old_node));
        return false;
    }

    node = node->NextSiblingElement();

    if (node != nullptr)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected other tag. Line " << PRINTLINE(node));
        return false;
    }

    return true;
}
