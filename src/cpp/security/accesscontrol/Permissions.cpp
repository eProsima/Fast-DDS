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

/*!
 * @file Permissions.cpp
 */

#include <cassert>
#include <fstream>

#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#define OPENSSL_CONST const
#else
#define IS_OPENSSL_1_1 0
#define OPENSSL_CONST
#endif // if OPENSSL_VERSION_NUMBER >= 0x10100000L

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/obj_mac.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/security/exceptions/SecurityException.h>
#include <security/accesscontrol/AccessPermissionsHandle.h>
#include <security/accesscontrol/DistinguishedName.h>
#include <security/accesscontrol/GovernanceParser.h>
#include <security/accesscontrol/Permissions.h>
#include <security/accesscontrol/PermissionsParser.h>
#include <security/artifact_providers/FileProvider.hpp>
#include <security/authentication/PKIIdentityHandle.h>
#include <security/logging/LogTopic.h>
#include <utils/StringMatching.hpp>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace security;

/**
 * @brief Convert a signature algortihm before adding it to a PermissionsToken.
 *
 * This methods converts the signature algorithm to the format used in the PermissionsToken.
 * Depending on the value of the use_legacy parameter, the algorithm will be converted to the legacy format or to the
 * one specified in the DDS-SEC 1.1 specification.
 *
 * @param algorithm The algorithm to convert.
 * @param use_legacy Whether to use the legacy format or not.
 *
 * @return The converted algorithm.
 */
static std::string convert_to_token_algo(
        const std::string& algorithm,
        bool use_legacy)
{
    // Leave as internal format when legacy is used
    if (use_legacy)
    {
        return algorithm;
    }

    // Convert to token format
    if (algorithm == RSA_SHA256)
    {
        return RSA_SHA256_FOR_TOKENS;
    }
    else if (algorithm == ECDSA_SHA256)
    {
        return ECDSA_SHA256_FOR_TOKENS;
    }

    return algorithm;
}

/**
 * @brief Parse a signature algorithm from a PermissionsToken.
 *
 * This method parses a signature algorithm from a PermissionsToken.
 * It converts the algorithm to the internal (legacy) format used by the library.
 *
 * @param algorithm The algorithm to parse.
 *
 * @return The parsed algorithm.
 */
static std::string parse_token_algo(
        const std::string& algorithm)
{
    // Convert to internal format, allowing both legacy and new formats
    if (algorithm == RSA_SHA256_FOR_TOKENS)
    {
        return RSA_SHA256;
    }
    else if (algorithm == ECDSA_SHA256_FOR_TOKENS)
    {
        return ECDSA_SHA256;
    }

    return algorithm;
}

static bool is_domain_in_set(
        const uint32_t domain_id,
        const Domains& domains)
{
    bool returned_value = false;

    for (auto range : domains.ranges)
    {
        if (range.second == 0)
        {
            if (domain_id == range.first)
            {
                returned_value = true;
                break;
            }
        }
        else
        {
            if (domain_id >= range.first &&
                    domain_id <= range.second)
            {
                returned_value = true;
                break;
            }
        }
    }

    return returned_value;
}

static const EndpointSecurityAttributes* is_topic_in_sec_attributes(
        const char* topic_name,
        const std::vector<std::pair<std::string, EndpointSecurityAttributes>>& attributes)
{
    const EndpointSecurityAttributes* returned_value = nullptr;

    for (auto& topic : attributes)
    {
        if (StringMatching::matchString(topic.first.c_str(), topic_name))
        {
            returned_value = &topic.second;
            break;
        }
    }

    return returned_value;
}

static bool is_topic_in_criterias(
        const char* topic_name,
        const std::vector<Criteria>& criterias)
{
    bool returned_value = false;

    for (auto criteria_it = criterias.begin(); !returned_value &&
            criteria_it != criterias.end(); ++criteria_it)
    {
        for (auto topic : (*criteria_it).topics)
        {
            if (StringMatching::matchPattern(topic.c_str(), topic_name))
            {
                returned_value = true;
                break;
            }
        }
    }

    return returned_value;
}

static bool is_partition_in_criterias(
        const std::string& partition,
        const std::vector<Criteria>& criterias)
{
    bool returned_value = false;

    for (auto criteria_it = criterias.begin(); !returned_value &&
            criteria_it != criterias.end(); ++criteria_it)
    {
        for (auto part : (*criteria_it).partitions)
        {
            if (StringMatching::matchPattern(part.c_str(), partition.c_str()))
            {
                returned_value = true;
                break;
            }
        }
    }

    return returned_value;
}

static bool check_rule(
        const char* topic_name,
        const Rule& rule,
        const std::vector<std::string>& partitions,
        const std::vector<Criteria>& criterias,
        SecurityException& exception)
{
    bool returned_value = false;

    if (rule.allow)
    {
        returned_value = true;

        if (partitions.empty())
        {
            if (!is_partition_in_criterias(std::string(), criterias))
            {
                returned_value = false;
                exception = _SecurityException_(std::string("<empty> partition not found in rule."));
            }
        }
        else
        {
            // Search partitions
            for (auto partition_it = partitions.begin(); returned_value && partition_it != partitions.end();
                    ++partition_it)
            {
                if (!is_partition_in_criterias(*partition_it, criterias))
                {
                    returned_value = false;
                    exception = _SecurityException_(*partition_it + std::string(" partition not found in rule."));
                }
            }
        }
    }
    else
    {
        exception = _SecurityException_(topic_name + std::string(" topic denied by deny rule."));
    }

    return returned_value;
}

static bool is_validation_in_time(
        const Validity& validity)
{
#if _MSC_VER != 1800
    bool returned_value = false;
    std::time_t current_time = std::time(nullptr);

    if (std::difftime(current_time, validity.not_before) >= 0)
    {
        if (std::difftime(validity.not_after, current_time) >= 0)
        {
            returned_value = true;
        }
    }

    return returned_value;
#else
    (void)validity;
    return true;
#endif // if _MSC_VER != 1800
}

static bool get_signature_algorithm(
        X509* certificate,
        std::string& signature_algorithm,
        SecurityException& exception)
{
    bool returnedValue = false;
    BUF_MEM* ptr = nullptr;
    OPENSSL_CONST X509_ALGOR* sigalg = nullptr;
    OPENSSL_CONST ASN1_BIT_STRING* sig = nullptr;

    BIO* out = BIO_new(BIO_s_mem());

    if (out != nullptr)
    {
        X509_get0_signature(&sig, &sigalg, certificate);

        if (sigalg != nullptr)
        {
            if (i2a_ASN1_OBJECT(out, sigalg->algorithm) > 0)
            {
                BIO_get_mem_ptr(out, &ptr);

                if (ptr != nullptr)
                {
                    if (strncmp(ptr->data, "ecdsa-with-SHA256", ptr->length) == 0)
                    {
                        signature_algorithm = ECDSA_SHA256;
                        returnedValue = true;
                    }
                    else if (strncmp(ptr->data, "sha256WithRSAEncryption", ptr->length) == 0)
                    {
                        signature_algorithm = RSA_SHA256;
                        returnedValue = true;
                    }
                    else if (strncmp(ptr->data, "sha1WithRSAEncryption", ptr->length) == 0)
                    {
                        signature_algorithm = RSA_SHA256;
                        returnedValue = true;
                    }
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr");
                }
            }
        }
        else
        {
            exception = _SecurityException_("OpenSSL library cannot write cert");
        }

        BIO_free(out);
    }
    else
    {
        exception = _SecurityException_("OpenSSL library cannot allocate mem");
    }

    return returnedValue;
}

// Auxiliary functions
static BIO* load_and_verify_document(
        X509_STORE* store,
        BIO* in,
        SecurityException& exception)
{
    BIO* out = nullptr;

    assert(nullptr != store);
    assert(nullptr != in);

    // Create PKCS7 object from input data
    BIO* indata = nullptr;
    PKCS7* p7 = SMIME_read_PKCS7(in, &indata);
    if (nullptr == p7)
    {
        exception = _SecurityException_("Input data has not PKCS7 S/MIME format");
        return nullptr;
    }

    // ---- Get certificate stack from store ----
    // The following lines are almost equivalent to the OpenSSL 3 API X509_STORE_get1_all_certs.
    // It creates a stack of X509 objects and populates the stack with the X509 objects contained in the store.
    STACK_OF(X509) * stack = sk_X509_new_null();
    if (nullptr == stack)
    {
        exception = _SecurityException_("Cannot allocate certificate stack");
    }
    else
    {
        STACK_OF(X509_OBJECT) * objects = X509_STORE_get0_objects(store);
        int i = 0;
        for (i = 0; i < sk_X509_OBJECT_num(objects); i++)
        {
            X509_OBJECT* tmp = sk_X509_OBJECT_value(objects, i);
            X509* cert = X509_OBJECT_get0_X509(tmp);
            if (nullptr != cert)
            {
                sk_X509_push(stack, cert);
            }
        }

        if (0 == sk_X509_num(stack))
        {
            exception = _SecurityException_("Certificate store should have at least one certificate");

            sk_X509_free(stack);
            stack = nullptr;
        }
    }
    // ---- Finished getting certificate stack from store ----

    if (nullptr != stack)
    {
        // Allocate output data
        out = BIO_new(BIO_s_mem());
        if (nullptr == out)
        {
            exception = _SecurityException_("Cannot allocate output BIO");
        }
        else
        {
            // Verify the input data using exclusively the certificates in the stack.
            // PKCS7_NOINTERN is used to ignore certificates coming alongside the signed data.
            // PKCS7_NOVERIFY is used since the permissions CA certificate will not be chain verified.
            if (!PKCS7_verify(p7, stack, nullptr, indata, out, PKCS7_TEXT | PKCS7_NOVERIFY | PKCS7_NOINTERN))
            {
                exception = _SecurityException_("PKCS7 data verification failed");
                BIO_free(out);
                out = nullptr;
            }
        }

        // Free the certificate stack
        sk_X509_free(stack);
    }

    PKCS7_free(p7);

    if (indata != nullptr)
    {
        BIO_free(indata);
    }

    return out;
}

static X509_STORE* load_permissions_ca(
        const std::string& permissions_ca,
        bool& there_are_crls,
        std::string& ca_sn,
        std::string& ca_algo,
        SecurityException& exception)
{
    if (permissions_ca.size() >= 7 && permissions_ca.compare(0, 7, "file://") == 0)
    {
        return detail::FileProvider::load_ca(permissions_ca, there_are_crls, ca_sn, ca_algo, get_signature_algorithm,
                       exception);
    }

    exception = _SecurityException_(std::string("Unsupported URI format ") + permissions_ca);
    return nullptr;
}

static BIO* load_signed_file(
        X509_STORE* store,
        std::string& file,
        SecurityException& exception)
{
    assert(store);
    BIO* out = nullptr;

    if (file.size() >= 7 && file.compare(0, 7, "file://") == 0)
    {
        BIO* in = BIO_new_file(file.substr(7).c_str(), "r");
        if (in != nullptr)
        {
            out = load_and_verify_document(store, in, exception);
            BIO_free(in);
        }
        else
        {
            exception = _SecurityException_(std::string("Cannot read file ") + file);
        }
    }
    else
    {
        exception = _SecurityException_(std::string("Unsupported governance file format ") + file);
    }

    return out;
}

static bool load_governance_file(
        AccessPermissionsHandle& ah,
        std::string& governance_file,
        DomainAccessRules& rules,
        SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, governance_file, exception);

    if (file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if (ptr != nullptr)
        {
            GovernanceParser parser;
            if ((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
            {
                parser.swap(rules);
            }
            else
            {
                exception = _SecurityException_(std::string("Malformed governance file ") + governance_file);
            }
        }
        else
        {
            exception = _SecurityException_(std::string("OpenSSL library cannot retrieve mem ptr from file ")
                            + governance_file);
        }

        BIO_free(file_mem);
    }

    return returned_value;
}

static bool load_permissions_file(
        AccessPermissionsHandle& ah,
        std::string& permissions_file,
        PermissionsData& permissions,
        SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, permissions_file, exception);

    if (file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if (ptr != nullptr)
        {
            PermissionsParser parser;
            if ((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
            {
                parser.swap(permissions);
            }
            else
            {
                exception = _SecurityException_(std::string("Malformed permissions file ") + permissions_file);
            }
        }
        else
        {
            exception = _SecurityException_(std::string("OpenSSL library cannot retrieve mem ptr from file ")
                            + permissions_file);
        }

        BIO_free(file_mem);
    }

    return returned_value;
}

static bool verify_permissions_file(
        const AccessPermissionsHandle& local_handle,
        const std::string& permissions_file,
        PermissionsData& permissions,
        SecurityException& exception)
{
    bool returned_value = false;

    if (permissions_file.size() <= static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        BIO* permissions_buf = BIO_new_mem_buf(permissions_file.data(), static_cast<int>(permissions_file.size()));
        if (permissions_buf != nullptr)
        {
            BIO* out = load_and_verify_document(local_handle->store_, permissions_buf, exception);
            if (nullptr != out)
            {
                BUF_MEM* ptr = nullptr;
                BIO_get_mem_ptr(out, &ptr);

                if (ptr != nullptr)
                {
                    PermissionsParser parser;
                    if ((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
                    {
                        parser.swap(permissions);
                        returned_value = true;
                    }
                    else
                    {
                        exception = _SecurityException_(std::string(
                                            "Malformed permissions file ") + permissions_file);
                    }
                }
                else
                {
                    exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr from file.");
                }
                BIO_free(out);
            }
            else
            {
                exception = _SecurityException_("Cannot read as PKCS7 the permissions file.");
            }

            BIO_free(permissions_buf);
        }
    }

    return returned_value;
}

static void process_protection_kind(
        const ProtectionKind kind,
        bool& protected_flag,
        bool& encrypted_flag,
        bool& orig_auth_flag)
{
    protected_flag = kind != ProtectionKind::NONE;
    encrypted_flag = (kind == ProtectionKind::ENCRYPT) || (kind == ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION);
    orig_auth_flag = (kind == ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION) ||
            (kind == ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION);
}

static bool check_subject_name(
        const IdentityHandle& ih,
        AccessPermissionsHandle& ah,
        const uint32_t domain_id,
        DomainAccessRules& governance,
        PermissionsData& permissions,
        SecurityException& exception)
{
    bool returned_value = false;
    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(ih);

    if (!lih.nil())
    {
        for (auto grant : permissions.grants)
        {
            if (is_validation_in_time(grant.validity))
            {
                if (rfc2253_string_compare(grant.subject_name, lih->cert_sn_rfc2253_))
                {
                    ah->grant = std::move(grant);
                    returned_value = true;

                    // Remove rules not apply to my domain
                    auto iterator = grant.rules.begin();
                    while (iterator != grant.rules.end())
                    {
                        if (!is_domain_in_set(domain_id, iterator->domains))
                        {
                            iterator = grant.rules.erase(iterator);
                        }
                        else
                        {
                            ++iterator;
                        }
                    }

                    break;
                }
            }
        }

        if (returned_value)
        {
            // Retry governance info.
            for (auto rule : governance.rules)
            {
                if (is_domain_in_set(domain_id, rule.domains))
                {
                    ah->governance_rule_.is_access_protected = rule.enable_join_access_control;

                    PluginParticipantSecurityAttributes plug_part_attr;

                    process_protection_kind(rule.discovery_protection_kind,
                            ah->governance_rule_.is_discovery_protected,
                            plug_part_attr.is_discovery_encrypted,
                            plug_part_attr.is_discovery_origin_authenticated);

                    process_protection_kind(rule.rtps_protection_kind,
                            ah->governance_rule_.is_rtps_protected,
                            plug_part_attr.is_rtps_encrypted,
                            plug_part_attr.is_rtps_origin_authenticated);

                    process_protection_kind(rule.liveliness_protection_kind,
                            ah->governance_rule_.is_liveliness_protected,
                            plug_part_attr.is_liveliness_encrypted,
                            plug_part_attr.is_liveliness_origin_authenticated);

                    if (rule.allow_unauthenticated_participants)
                    {
                        if (ah->governance_rule_.is_rtps_protected)
                        {
                            exception = _SecurityException_(
                                "allow_unauthenticated_participants cannot be enabled if rtps_protection_kind is not none");
                            returned_value = false;
                            break;
                        }
                        else
                        {
                            ah->governance_rule_.allow_unauthenticated_participants = true;
                        }
                    }

                    ah->governance_rule_.plugin_participant_attributes = plug_part_attr.mask();

                    for (auto topic_rule : rule.topic_rules)
                    {
                        std::string topic_expression = topic_rule.topic_expression;
                        EndpointSecurityAttributes security_attributes;
                        PluginEndpointSecurityAttributes plugin_attributes;

                        security_attributes.is_discovery_protected = topic_rule.enable_discovery_protection;
                        security_attributes.is_liveliness_protected = topic_rule.enable_liveliness_protection;
                        security_attributes.is_read_protected = topic_rule.enable_read_access_control;
                        security_attributes.is_write_protected = topic_rule.enable_write_access_control;

                        bool hasEncryption =
                                (topic_rule.metadata_protection_kind == ProtectionKind::ENCRYPT) ||
                                (topic_rule.metadata_protection_kind ==
                                ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION);
                        bool hasOriginAuth =
                                (topic_rule.metadata_protection_kind ==
                                ProtectionKind::ENCRYPT_WITH_ORIGIN_AUTHENTICATION) ||
                                (topic_rule.metadata_protection_kind ==
                                ProtectionKind::SIGN_WITH_ORIGIN_AUTHENTICATION);
                        plugin_attributes.is_submessage_encrypted = hasEncryption;
                        plugin_attributes.is_submessage_origin_authenticated = hasOriginAuth;

                        security_attributes.is_submessage_protected =
                                (topic_rule.metadata_protection_kind != ProtectionKind::NONE);

                        plugin_attributes.is_payload_encrypted =
                                security_attributes.is_key_protected =
                                (topic_rule.data_protection_kind == ProtectionKind::ENCRYPT);
                        security_attributes.is_payload_protected =
                                (topic_rule.data_protection_kind != ProtectionKind::NONE);

                        security_attributes.plugin_endpoint_attributes = plugin_attributes.mask();

                        ah->governance_topic_rules_.push_back(std::pair<std::string, EndpointSecurityAttributes>(
                                    std::move(topic_expression), std::move(security_attributes)));
                    }

                    break;
                }
            }
        }
        else
        {
            exception =
                    _SecurityException_(std::string(
                                "Not found the identity subject name in permissions file. Subject name: ") +
                            lih->cert_sn_rfc2253_);
        }
    }
    else
    {
        exception = _SecurityException_("IdentityHandle is not of the type PKIIdentityHandle");
    }

    return returned_value;
}

static bool generate_permissions_token(
        AccessPermissionsHandle& handle,
        bool transmit_legacy_algorithms)
{
    Property property;
    PermissionsToken& token = handle->permissions_token_;
    token.class_id("DDS:Access:Permissions:1.0");

    property.name("dds.perm_ca.sn");
    property.value() = handle->sn;
    property.propagate(true);
    token.properties().push_back(std::move(property));

    property.name("dds.perm_ca.algo");
    property.value() = convert_to_token_algo(handle->algo, transmit_legacy_algorithms);
    property.propagate(true);
    token.properties().push_back(std::move(property));

    return true;
}

static bool generate_credentials_token(
        AccessPermissionsHandle& handle,
        const std::string& file,
        SecurityException& exception)
{
    bool returned_value = false;
    // Create PermissionsCredentialToken;
    PermissionsCredentialToken& token = handle->permissions_credential_token_;
    token.class_id("DDS:Access:PermissionsCredential");

    if (file.size() >= 7 && file.compare(0, 7, "file://") == 0)
    {
        try
        {
            std::ifstream ifs(file.substr(7).c_str());
            Property property;
            property.name("dds.perm.cert");
            property.value().assign((std::istreambuf_iterator<char>(ifs)),
                    (std::istreambuf_iterator<char>()));
            property.propagate(true);
            token.properties().push_back(std::move(property));
            returned_value = true;
        }
        catch (std::exception&)
        {
            exception = _SecurityException_(std::string("Cannot find file ") + file);
        }
    }
    else
    {
        exception = _SecurityException_("Unsupported permissions_ca format");
    }

    return returned_value;
}

PermissionsHandle* Permissions::validate_local_permissions(
        Authentication&,
        const IdentityHandle& identity,
        const uint32_t domain_id,
        const RTPSParticipantAttributes& participant_attr,
        SecurityException& exception)
{
    PropertyPolicy access_properties = PropertyPolicyHelper::get_properties_with_prefix(participant_attr.properties,
                    "dds.sec.access.builtin.Access-Permissions.");

    if (PropertyPolicyHelper::length(access_properties) == 0)
    {
        exception = _SecurityException_("Not found any dds.sec.access.builtin.Access-Permissions property");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    bool transmit_legacy_algorithms = false;
    std::string* legacy = PropertyPolicyHelper::find_property(access_properties, "transmit_algorithms_as_legacy");
    if (legacy != nullptr)
    {
        transmit_legacy_algorithms = (*legacy == "true");
    }

    std::string* permissions_ca = PropertyPolicyHelper::find_property(access_properties, "permissions_ca");

    if (permissions_ca == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.permissions_ca property");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    std::string* governance = PropertyPolicyHelper::find_property(access_properties, "governance");

    if (governance == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.governance property");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    std::string* permissions = PropertyPolicyHelper::find_property(access_properties, "permissions");

    if (permissions == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.permissions property");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    AccessPermissionsHandle* ah = &AccessPermissionsHandle::narrow(*get_permissions_handle(exception));

    (*ah)->store_ = load_permissions_ca(*permissions_ca, (*ah)->there_are_crls_, (*ah)->sn, (*ah)->algo, exception);

    if ((*ah)->store_ != nullptr)
    {
        DomainAccessRules rules;
        if (load_governance_file(*ah, *governance, rules, exception))
        {
            PermissionsData permissions_data;
            if (load_permissions_file(*ah, *permissions, permissions_data, exception))
            {
                // Check subject name.
                if (check_subject_name(identity, *ah, domain_id, rules, permissions_data, exception))
                {
                    if (generate_permissions_token(*ah, transmit_legacy_algorithms))
                    {
                        if (generate_credentials_token(*ah, *permissions, exception))
                        {
                            return ah;
                        }
                    }
                }
            }
        }
    }

    delete ah;

    return nullptr;
}

bool Permissions::get_permissions_token(
        PermissionsToken** permissions_token,
        const PermissionsHandle& handle,
        SecurityException& exception)
{
    const AccessPermissionsHandle& phandle = AccessPermissionsHandle::narrow(handle);

    if (!phandle.nil())
    {
        *permissions_token = new PermissionsToken(phandle->permissions_token_);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid permissions handle");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return false;
}

bool Permissions::return_permissions_token(
        PermissionsToken* token,
        SecurityException& /*exception*/)
{
    delete token;
    return true;
}

bool Permissions::get_permissions_credential_token(
        PermissionsCredentialToken** permissions_credential_token,
        const PermissionsHandle& handle,
        SecurityException& exception)
{
    const AccessPermissionsHandle& phandle = AccessPermissionsHandle::narrow(handle);

    if (!phandle.nil())
    {
        *permissions_credential_token = new PermissionsCredentialToken(phandle->permissions_credential_token_);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid permissions handle");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return false;
}

bool Permissions::return_permissions_credential_token(
        PermissionsCredentialToken* token,
        SecurityException&)
{
    delete token;
    return true;
}

PermissionsHandle* Permissions::get_permissions_handle(
        SecurityException&)
{
    return new (std::nothrow) AccessPermissionsHandle();
}

bool Permissions::return_permissions_handle(
        PermissionsHandle* permissions_handle,
        SecurityException&)
{
    AccessPermissionsHandle* handle = &AccessPermissionsHandle::narrow(*permissions_handle);

    if (!handle->nil())
    {
        delete handle;
        return true;
    }

    return false;
}

PermissionsHandle* Permissions::validate_remote_permissions(
        Authentication&,
        const IdentityHandle& local_identity_handle,
        const PermissionsHandle& local_permissions_handle,
        const IdentityHandle& remote_identity_handle,
        const PermissionsToken& remote_permissions_token,
        const PermissionsCredentialToken& remote_credential_token,
        SecurityException& exception)
{
    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(local_identity_handle);
    const AccessPermissionsHandle& lph = AccessPermissionsHandle::narrow(local_permissions_handle);
    const PKIIdentityHandle& rih = PKIIdentityHandle::narrow(remote_identity_handle);

    if (lih.nil() || lph.nil() || rih.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    // Check permissions.
    // Check c.id
    const std::string* sn = DataHolderHelper::find_property_value(remote_permissions_token, "dds.perm_ca.sn");

    if (sn != nullptr)
    {
        if (sn->compare(lph->sn) != 0)
        {
            exception = _SecurityException_("Remote participant PermissionsCA differs from local");
            EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
            return nullptr;
        }
    }

    const std::string* algo = DataHolderHelper::find_property_value(remote_permissions_token, "dds.perm_ca.algo");

    if (algo != nullptr)
    {
        std::string used_algo = parse_token_algo(*algo);
        if (used_algo.compare(lph->algo) != 0)
        {
            exception = _SecurityException_("Remote participant PermissionsCA algorithm differs from local");
            EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
            return nullptr;
        }
    }

    const std::string* permissions_file = DataHolderHelper::find_property_value(remote_credential_token,
                    "dds.perm.cert");

    if (permissions_file == nullptr)
    {
        exception = _SecurityException_("Remote participant doesn't sent the signed permissions file");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    PermissionsData data;
    if (!verify_permissions_file(lph, *permissions_file, data, exception))
    {
        return nullptr;
    }

    Grant remote_grant;
    for (auto grant : data.grants)
    {
        if (is_validation_in_time(grant.validity))
        {
            if (rfc2253_string_compare(grant.subject_name, rih->cert_sn_rfc2253_) ||
                    strcmp(grant.subject_name.c_str(), rih->cert_sn_.c_str()) == 0)
            {
                remote_grant = std::move(grant);
                break;
            }
        }
    }

    if (remote_grant.subject_name.empty())
    {
        exception = _SecurityException_("Remote participant doesn't found in its permissions file");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return nullptr;
    }

    AccessPermissionsHandle* handle = &AccessPermissionsHandle::narrow(*get_permissions_handle(exception));
    (*handle)->grant = std::move(remote_grant);
    (*handle)->governance_rule_ = lph->governance_rule_;
    (*handle)->governance_topic_rules_ = lph->governance_topic_rules_;

    return handle;
}

bool Permissions::check_create_participant(
        const PermissionsHandle& local_handle,
        const uint32_t /*domain_id*/,
        const RTPSParticipantAttributes&,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if (lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    //Search an allow rule with my domain
    for (auto rule : lah->grant.rules)
    {
        if (rule.allow)
        {
            returned_value = true;
            break;
        }
    }

    if (!returned_value)
    {
        exception = _SecurityException_("Not found a rule allowing to use the domain_id");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::check_remote_participant(
        const PermissionsHandle& remote_handle,
        const uint32_t domain_id,
        const ParticipantProxyData&,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);

    if (rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    if (!rah->governance_rule_.is_access_protected)
    {
        return true;
    }

    //Search an allow rule with my domain
    for (auto rule : rah->grant.rules)
    {
        if (rule.allow)
        {
            if (is_domain_in_set(domain_id, rule.domains))
            {
                returned_value = true;
                break;
            }
        }
    }

    if (!returned_value)
    {
        exception = _SecurityException_("Not found a rule allowing to use the domain_id");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::check_create_datawriter(
        const PermissionsHandle& local_handle,
        const uint32_t /*domain_id*/,
        const std::string& topic_name,
        const std::vector<std::string>& partitions,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if (lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    const EndpointSecurityAttributes* attributes = nullptr;

    if ((attributes = is_topic_in_sec_attributes(topic_name.c_str(), lah->governance_topic_rules_)) != nullptr)
    {
        if (!attributes->is_write_protected)
        {
            return true;
        }
    }
    else
    {
        exception = _SecurityException_("Not found topic access rule for topic " + topic_name);
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    // Search topic
    for (auto rule : lah->grant.rules)
    {
        if (is_topic_in_criterias(topic_name.c_str(), rule.publishes))
        {
            returned_value = check_rule(topic_name.c_str(), rule, partitions, rule.publishes, exception);
            break;
        }
    }

    if (!returned_value)
    {
        if (strlen(exception.what()) == 0)
        {
            exception = _SecurityException_(topic_name + std::string(" topic not found in allow rule."));
        }
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::check_create_datareader(
        const PermissionsHandle& local_handle,
        const uint32_t /*domain_id*/,
        const std::string& topic_name,
        const std::vector<std::string>& partitions,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if (lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    const EndpointSecurityAttributes* attributes = nullptr;

    if ((attributes = is_topic_in_sec_attributes(topic_name.c_str(), lah->governance_topic_rules_)) != nullptr)
    {
        if (!attributes->is_read_protected)
        {
            return true;
        }
    }
    else
    {
        exception = _SecurityException_("Not found topic access rule for topic " + topic_name);
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    for (auto rule : lah->grant.rules)
    {
        if (is_topic_in_criterias(topic_name.c_str(), rule.subscribes))
        {
            returned_value = check_rule(topic_name.c_str(), rule, partitions, rule.subscribes, exception);
            break;
        }
    }

    if (!returned_value)
    {
        if (strlen(exception.what()) == 0)
        {
            exception = _SecurityException_(topic_name + std::string(" topic not found in allow rule."));
        }
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::check_remote_datawriter(
        const PermissionsHandle& remote_handle,
        const uint32_t domain_id,
        const WriterProxyData& publication_data,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);
    const char* topic_name = publication_data.topic_name.c_str();

    if (rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    const EndpointSecurityAttributes* attributes = nullptr;

    if ((attributes = is_topic_in_sec_attributes(topic_name, rah->governance_topic_rules_))
            != nullptr)
    {
        if (!attributes->is_write_protected)
        {
            return true;
        }
    }
    else
    {
        exception = _SecurityException_(
            "Not found topic access rule for topic " + publication_data.topic_name.to_string());
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    for (auto rule : rah->grant.rules)
    {
        if (is_domain_in_set(domain_id, rule.domains))
        {
            if (is_topic_in_criterias(topic_name, rule.publishes))
            {
                returned_value = check_rule(topic_name, rule, publication_data.partition.getNames(),
                                rule.publishes, exception);
                break;
            }
        }
    }

    if (!returned_value)
    {
        if (strlen(exception.what()) == 0)
        {
            exception = _SecurityException_(topic_name + std::string(" topic not found in allow rule."));
        }
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::check_remote_datareader(
        const PermissionsHandle& remote_handle,
        const uint32_t domain_id,
        const ReaderProxyData& subscription_data,
        bool& relay_only,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);
    const char* topic_name = subscription_data.topic_name.c_str();

    relay_only = false;

    if (rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    const EndpointSecurityAttributes* attributes = nullptr;

    if ((attributes = is_topic_in_sec_attributes(topic_name, rah->governance_topic_rules_))
            != nullptr)
    {
        if (!attributes->is_read_protected)
        {
            return true;
        }
    }
    else
    {
        exception = _SecurityException_(
            "Not found topic access rule for topic " + subscription_data.topic_name.to_string());
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    for (auto rule : rah->grant.rules)
    {
        if (is_domain_in_set(domain_id, rule.domains))
        {
            const std::vector<std::string>& partitions = subscription_data.partition.getNames();
            if (is_topic_in_criterias(topic_name, rule.subscribes))
            {
                returned_value = check_rule(topic_name, rule, partitions, rule.subscribes, exception);
                break;
            }

            if (is_topic_in_criterias(topic_name, rule.relays))
            {
                returned_value = check_rule(topic_name, rule, partitions, rule.relays, exception);
                if (returned_value)
                {
                    relay_only = true;
                }

                break;
            }
        }
    }

    if (!returned_value)
    {
        if (strlen(exception.what()) == 0)
        {
            exception = _SecurityException_(topic_name + std::string(" topic not found in allow rule."));
        }
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return returned_value;
}

bool Permissions::get_participant_sec_attributes(
        const PermissionsHandle& local_handle,
        ParticipantSecurityAttributes& attributes,
        SecurityException& exception)
{
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if (lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
        return false;
    }

    attributes = lah->governance_rule_;
    return true;
}

bool Permissions::get_datawriter_sec_attributes(
        const PermissionsHandle& permissions_handle,
        const std::string& topic_name,
        const std::vector<std::string>& /*partitions*/,
        EndpointSecurityAttributes& attributes,
        SecurityException& exception)
{
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(permissions_handle);
    const EndpointSecurityAttributes* attr = nullptr;

    if ((attr = is_topic_in_sec_attributes(topic_name.c_str(), lah->governance_topic_rules_))
            != nullptr)
    {
        attributes = *attr;
        return true;
    }
    else
    {
        exception = _SecurityException_("Not found topic access rule for topic " + topic_name);
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return false;
}

bool Permissions::get_datareader_sec_attributes(
        const PermissionsHandle& permissions_handle,
        const std::string& topic_name,
        const std::vector<std::string>& /*partitions*/,
        EndpointSecurityAttributes& attributes,
        SecurityException& exception)
{
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(permissions_handle);
    const EndpointSecurityAttributes* attr = nullptr;

    if ((attr = is_topic_in_sec_attributes(topic_name.c_str(), lah->governance_topic_rules_))
            != nullptr)
    {
        attributes = *attr;
        return true;
    }
    else
    {
        exception = _SecurityException_("Not found topic access rule for topic " + topic_name);
        EMERGENCY_SECURITY_LOGGING("Permissions", exception.what());
    }

    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
