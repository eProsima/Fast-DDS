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

#include "Permissions.h"
#include "AccessPermissionsHandle.h"
#include "GovernanceParser.h"
#include "PermissionsParser.h"
#include "../authentication/PKIIdentityHandle.h"
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/security/exceptions/SecurityException.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/utils/StringMatching.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>

#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define IS_OPENSSL_1_1 1
#define OPENSSL_CONST const
#else
#define IS_OPENSSL_1_1 0
#define OPENSSL_CONST
#endif

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/obj_mac.h>

#include <cassert>
#include <fstream>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

static bool is_domain_in_set(const uint32_t domain_id, const Domains& domains)
{
    bool returned_value = false;

    for(auto range : domains.ranges)
    {
        if(range.second == 0)
        {
            if(domain_id == range.first)
            {
                returned_value = true;
                break;
            }
        }
        else
        {
            if(domain_id >= range.first &&
                    domain_id <= range.second)
            {
                returned_value = true;
                break;
            }
        }
    }

    return returned_value;
}

static bool is_topic_in_criterias(const std::string& topic_name, const std::vector<Criteria>& criterias)
{
    bool returned_value = false;

    for(auto criteria_it = criterias.begin(); !returned_value &&
            criteria_it != criterias.end(); ++criteria_it)
    {
        for(auto topic : (*criteria_it).topics)
        {
            if(StringMatching::matchString(topic.c_str(), topic_name.c_str()))
            {
                returned_value = true;
                break;
            }
        }
    }

    return returned_value;
}

static bool get_signature_algorithm(X509* certificate, std::string& signature_algorithm, SecurityException& exception)
{
    bool returnedValue = false;
    BUF_MEM* ptr = nullptr;
    OPENSSL_CONST X509_ALGOR* sigalg = nullptr;
    OPENSSL_CONST ASN1_BIT_STRING* sig = nullptr;

    BIO *out = BIO_new(BIO_s_mem());

    if(out != nullptr)
    {
        X509_get0_signature(&sig, &sigalg, certificate);

        if(sigalg != nullptr)
        {
            if(i2a_ASN1_OBJECT(out, sigalg->algorithm) > 0)
            {
                BIO_get_mem_ptr(out, &ptr);

                if(ptr != nullptr)
                {
                    if(strncmp(ptr->data, "ecdsa-with-SHA256", ptr->length) == 0)
                    {
                        signature_algorithm = ECDSA_SHA256;
                        returnedValue = true;
                    }
                }
                else
                    exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr");
            }
        }
        else
            exception = _SecurityException_("OpenSSL library cannot write cert");

        BIO_free(out);
    }
    else
        exception = _SecurityException_("OpenSSL library cannot allocate mem");

    return returnedValue;
}

// Auxiliary functions
static X509_STORE* load_permissions_ca(const std::string& permissions_ca, bool& there_are_crls,
        std::string& ca_sn, std::string& ca_algo, SecurityException& exception)
{
    X509_STORE* store = X509_STORE_new();

    if(store != nullptr)
    {
        if(permissions_ca.size() >= 7 && permissions_ca.compare(0, 7, "file://") == 0)
        {
            BIO* in = BIO_new(BIO_s_file());

            if(in != nullptr)
            {
                if(BIO_read_filename(in, permissions_ca.substr(7).c_str()) > 0)
                {
                    STACK_OF(X509_INFO)* inf = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL);

                    if(inf != nullptr)
                    {
                        int i, count = 0;
                        there_are_crls = false;

                        for (i = 0; i < sk_X509_INFO_num(inf); i++)
                        {
                            X509_INFO* itmp = sk_X509_INFO_value(inf, i);

                            if (itmp->x509)
                            {
                                // Retrieve subject name for future use.
                                if(ca_sn.empty())
                                {
                                    X509_NAME* ca_subject_name = X509_get_subject_name(itmp->x509);
                                    assert(ca_subject_name != nullptr);
                                    char* ca_subject_name_str = X509_NAME_oneline(ca_subject_name, 0, 0);
                                    assert(ca_subject_name_str != nullptr);
                                    ca_sn = ca_subject_name_str;
                                    OPENSSL_free(ca_subject_name_str);
                                }

                                // Retrieve signature algorithm
                                if(ca_algo.empty())
                                {
                                    if(get_signature_algorithm(itmp->x509, ca_algo, exception))
                                    {
                                        X509_STORE_add_cert(store, itmp->x509);
                                        count++;
                                    }
                                }
                                else
                                {
                                    X509_STORE_add_cert(store, itmp->x509);
                                    count++;
                                }
                            }
                            if (itmp->crl)
                            {
                                X509_STORE_add_crl(store, itmp->crl);
                                there_are_crls = true;
                            }
                        }

                        if(count > 0)
                        {
                            sk_X509_INFO_pop_free(inf, X509_INFO_free);
                            BIO_free(in);

                            return store;
                        }
                    }
                    else
                    {
                        exception = _SecurityException_(std::string("OpenSSL library cannot read X509 info in file ") + permissions_ca.substr(7));
                    }

                    sk_X509_INFO_pop_free(inf, X509_INFO_free);
                }
                else
                {
                    exception = _SecurityException_(std::string("OpenSSL library cannot read file ") + permissions_ca.substr(7));
                }

                BIO_free(in);
            }
            else
            {
                exception = _SecurityException_("OpenSSL library cannot allocate file");
            }
        }
        else
        {
            exception = _SecurityException_("Unsupported permissions_ca format");
        }

        X509_STORE_free(store);
    }
    else
    {
        exception = _SecurityException_("Creation of X509 storage");
    }

    return nullptr;
}

static BIO* load_signed_file(X509_STORE* store, std::string& file, SecurityException& exception)
{
    assert(store);
    BIO* out = nullptr;

    if(file.size() >= 7 && file.compare(0, 7, "file://") == 0)
    {
        BIO* in = BIO_new_file(file.substr(7).c_str(), "r");

        if(in != nullptr)
        {
            BIO* indata = nullptr;
            PKCS7* p7 = SMIME_read_PKCS7(in, &indata);

            if(p7 != nullptr)
            {
                out = BIO_new(BIO_s_mem());
                if(!PKCS7_verify(p7, nullptr, store, indata, out, PKCS7_TEXT))
                {
                    exception = _SecurityException_(std::string("Failed verification of the file ") + file);
                    BIO_free(out);
                    out = nullptr;
                }

                BIO_free(indata);
                PKCS7_free(p7);
            }
            else
            {
                exception = _SecurityException_(std::string("Cannot read as PKCS7 the file ") + file);
            }

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

static bool load_governance_file(AccessPermissionsHandle& ah, std::string& governance_file, DomainAccessRules& rules,
        SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, governance_file, exception);

    if(file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if(ptr != nullptr)
        {
            GovernanceParser parser;
            if((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
            {
                parser.swap(rules);
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

static bool load_permissions_file(AccessPermissionsHandle& ah, std::string& permissions_file,
        PermissionsData& permissions, SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, permissions_file, exception);

    if(file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if(ptr != nullptr)
        {
            PermissionsParser parser;
            if((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
            {
                parser.swap(permissions);
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

static bool verify_permissions_file(const AccessPermissionsHandle& local_handle, const std::string& permissions_file,
        PermissionsData& permissions, SecurityException& exception)
{
    bool returned_value = false;

    if(permissions_file.size() <= static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        BIO* permissions_buf = BIO_new_mem_buf(permissions_file.data(), static_cast<int>(permissions_file.size()));

        if(permissions_buf != nullptr)
        {
            BIO* indata = nullptr;
            PKCS7* p7 = SMIME_read_PKCS7(permissions_buf, &indata);

            if(p7 != nullptr)
            {
                BIO* out = BIO_new(BIO_s_mem());
                if(PKCS7_verify(p7, nullptr, local_handle->store_, indata, out, PKCS7_TEXT))
                {
                    BUF_MEM* ptr = nullptr;
                    BIO_get_mem_ptr(out, &ptr);

                    if(ptr != nullptr)
                    {
                        PermissionsParser parser;
                        if((returned_value = parser.parse_stream(ptr->data, ptr->length)) == true)
                        {
                            parser.swap(permissions);
                            returned_value = true;
                        }
                    }
                    else
                    {
                        exception = _SecurityException_("OpenSSL library cannot retrieve mem ptr from file.");
                    }
                }
                else
                {
                    exception = _SecurityException_("Failed verification of the permissions file");
                }

                BIO_free(out);
                BIO_free(indata);
                PKCS7_free(p7);
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

static bool check_subject_name(const IdentityHandle& ih, AccessPermissionsHandle& ah, PermissionsData& permissions,
        SecurityException& exception)
{
    bool returned_value = false;
    const PKIIdentityHandle& lih = PKIIdentityHandle::narrow(ih);

    if(!lih.nil())
    {
        for(auto grant : permissions.grants)
        {
            if(grant.subject_name.compare(lih->cert_sn_) == 0)
            {
                ah->grant = std::move(grant);
                returned_value = true;
            }
        }

        if(!returned_value)
        {
            exception = _SecurityException_(std::string("Not found the identity subject name in permissions file. Subject name: ") +
                    lih->cert_sn_);
        }
    }
    else
    {
        exception = _SecurityException_("IdentityHandle is not of the type PKIIdentityHandle");
    }

    return returned_value;
}

static bool generate_permissions_token(AccessPermissionsHandle& handle)
{
    Property property;
    PermissionsToken& token = handle->permissions_token_;
    token.class_id("DDS:Access:Permissions:1.0");

    property.name("dds.perm_ca.sn");
    property.value() = handle->sn;
    property.propagate(true);
    token.properties().push_back(std::move(property));

    property.name("dds.perm_ca.algo");
    property.value() = handle->algo;
    property.propagate(true);
    token.properties().push_back(std::move(property));

    return true;
}

static bool generate_credentials_token(AccessPermissionsHandle& handle, const std::string& file,
        SecurityException& exception)
{
    bool returned_value = false;
    // Create PermissionsCredentialToken;
    Property property;
    PermissionsCredentialToken& token = handle->permissions_credential_token_;
    token.class_id("DDS:Access:PermissionsCredential");

    if(file.size() >= 7 && file.compare(0, 7, "file://") == 0)
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
        catch(std::exception& ex)
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

PermissionsHandle* Permissions::validate_local_permissions(Authentication&,
        const IdentityHandle& identity,
        const uint32_t domain_id,
        const RTPSParticipantAttributes& participant_attr,
        SecurityException& exception)
{
    PropertyPolicy access_properties = PropertyPolicyHelper::get_properties_with_prefix(participant_attr.properties, "dds.sec.access.builtin.Access-Permissions.");

    if(PropertyPolicyHelper::length(access_properties) == 0)
    {
        exception = _SecurityException_("Not found any dds.sec.access.builtin.Access-Permissions property");
        return nullptr;
    }

    std::string* permissions_ca = PropertyPolicyHelper::find_property(access_properties, "permissions_ca");

    if(permissions_ca == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.permissions_ca property");
        return nullptr;
    }

    std::string* governance = PropertyPolicyHelper::find_property(access_properties, "governance");

    if(governance == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.governance property");
        return nullptr;
    }

    std::string* permissions = PropertyPolicyHelper::find_property(access_properties, "permissions");

    if(permissions == nullptr)
    {
        exception = _SecurityException_("Not found dds.sec.access.builtin.Access-Permissions.permissions property");
        return nullptr;
    }

    AccessPermissionsHandle* ah = new AccessPermissionsHandle();

    (*ah)->store_ = load_permissions_ca(*permissions_ca, (*ah)->there_are_crls_, (*ah)->sn, (*ah)->algo, exception);

    if((*ah)->store_ != nullptr)
    {
        DomainAccessRules rules;
        if(load_governance_file(*ah, *governance, rules, exception))
        {
            PermissionsData permissions_data;
            if(load_permissions_file(*ah, *permissions, permissions_data, exception))
            {
                // Check subject name.
                if(check_subject_name(identity, *ah, permissions_data, exception))
                {
                    if(generate_permissions_token(*ah))
                    {
                        if(generate_credentials_token(*ah, *permissions, exception))
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

bool Permissions::get_permissions_token(PermissionsToken** permissions_token,
        const PermissionsHandle& handle,
        SecurityException& exception)
{
    const AccessPermissionsHandle& phandle = AccessPermissionsHandle::narrow(handle);

    if(!phandle.nil())
    {
        *permissions_token = new PermissionsToken(phandle->permissions_token_);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid permissions handle");
    }

    return false;
}

bool Permissions::return_permissions_token(PermissionsToken* token,
        SecurityException& /*exception*/)
{
    delete token;
    return true;
}

bool Permissions::get_permissions_credential_token(PermissionsCredentialToken** permissions_credential_token,
        const PermissionsHandle& handle, SecurityException& exception)
{
    const AccessPermissionsHandle& phandle = AccessPermissionsHandle::narrow(handle);

    if(!phandle.nil())
    {
        *permissions_credential_token = new PermissionsCredentialToken(phandle->permissions_credential_token_);
        return true;
    }
    else
    {
        exception = _SecurityException_("Invalid permissions handle");
    }

    return false;
}

bool Permissions::return_permissions_credential_token(PermissionsCredentialToken* token,
        SecurityException&)
{
    delete token;
    return true;
}

bool Permissions::return_permissions_handle(PermissionsHandle* permissions_handle,
                SecurityException&)
{
    AccessPermissionsHandle* handle = &AccessPermissionsHandle::narrow(*permissions_handle);

    if(!handle->nil())
    {
        delete handle;
        return true;
    }

    return false;
}

PermissionsHandle* Permissions::validate_remote_permissions(Authentication&,
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

    if(lih.nil() || lph.nil() || rih.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return nullptr;
    }

    // Check permissions.
    // Check c.id
    const std::string* sn = DataHolderHelper::find_property_value(remote_permissions_token, "dds.perm_ca.sn");

    if(sn != nullptr)
    {
        if(sn->compare(lph->sn) != 0)
        {
            exception = _SecurityException_("Remote participant PermissionsCA differs from local");
            return nullptr;
        }
    }

    const std::string* algo = DataHolderHelper::find_property_value(remote_permissions_token, "dds.perm_ca.algo");

    if(algo != nullptr)
    {
        if(algo->compare(lph->algo) != 0)
        {
            exception = _SecurityException_("Remote participant PermissionsCA algorithm differs from local");
            return nullptr;
        }
    }

    const std::string* permissions_file = DataHolderHelper::find_property_value(remote_credential_token,
            "dds.perm.cert");

    if(permissions_file == nullptr)
    {
        exception = _SecurityException_("Remote participant doesn't sent the signed permissions file");
        return nullptr;
    }

    PermissionsData data;
    if(!verify_permissions_file(lph, *permissions_file, data, exception))
    {
        return nullptr;
    }

    Grant remote_grant;
    for(auto grant : data.grants)
    {
        if(grant.subject_name.compare(rih->cert_sn_) == 0)
        {
            remote_grant = std::move(grant);
            break;
        }
    }

    if(remote_grant.subject_name.empty())
    {
        exception = _SecurityException_("Remote participant doesn't found in its permissions file");
        return nullptr;
    }

    AccessPermissionsHandle* handle =  new AccessPermissionsHandle();
    (*handle)->grant = std::move(remote_grant);

    return handle;
}

bool Permissions::check_create_participant(const PermissionsHandle& local_handle, const uint32_t domain_id,
                const RTPSParticipantAttributes&, SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if(lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    //Search an allow rule with my domain
    for(auto rule : lah->grant.rules)
    {
        if(rule.allow)
        {
            if(is_domain_in_set(domain_id, rule.domains))
            {
                returned_value = true;
                break;
            }
        }
    }

    if(!returned_value)
    {
        exception = _SecurityException_("Not found a rule allowing to use the domain_id");
    }

    return returned_value;
}

bool Permissions::check_remote_participant(const PermissionsHandle& remote_handle, const uint32_t domain_id,
                const ParticipantProxyData&, SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);

    if(rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    //Search an allow rule with my domain
    for(auto rule : rah->grant.rules)
    {
        if(rule.allow)
        {
            if(is_domain_in_set(domain_id, rule.domains))
            {
                returned_value = true;
                break;
            }
        }
    }

    if(!returned_value)
    {
        exception = _SecurityException_("Not found a rule allowing to use the domain_id");
    }

    return returned_value;
}

bool Permissions::check_create_datawriter(const PermissionsHandle& local_handle,
        const uint32_t domain_id, const std::string& topic_name,
        const std::string& partitions, SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if(lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    for(auto rule : lah->grant.rules)
    {
        if(is_domain_in_set(domain_id, rule.domains))
        {
            if(is_topic_in_criterias(topic_name, rule.publishes))
            {
                if(rule.allow)
                {
                    returned_value = true;
                }
                else
                {
                    exception = _SecurityException_(topic_name +
                            std::string(" topic denied by deny rule."));
                }

                break;
            }
        }
    }

    if(!returned_value && strlen(exception.what()) == 0)
    {
        exception = _SecurityException_(topic_name +
                std::string(" topic not found in allow rule."));
    }

    return returned_value;
}

bool Permissions::check_create_datareader(const PermissionsHandle& local_handle,
        const uint32_t domain_id, const std::string& topic_name,
        const std::string& partitions, SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& lah = AccessPermissionsHandle::narrow(local_handle);

    if(lah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    for(auto rule : lah->grant.rules)
    {
        if(is_domain_in_set(domain_id, rule.domains))
        {
            if(is_topic_in_criterias(topic_name, rule.subscribes))
            {
                if(rule.allow)
                {
                    returned_value = true;
                }
                else
                {
                    exception = _SecurityException_(topic_name +
                            std::string(" topic denied by deny rule."));
                }

                break;
            }
        }
    }

    if(!returned_value && strlen(exception.what()) == 0)
    {
        exception = _SecurityException_(topic_name +
                std::string(" topic not found in allow rule."));
    }

    return returned_value;
}

bool Permissions::check_remote_datawriter(const PermissionsHandle& remote_handle,
        const uint32_t domain_id, const WriterProxyData& publication_data,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);

    if(rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    for(auto rule : rah->grant.rules)
    {
        if(is_domain_in_set(domain_id, rule.domains))
        {
            if(is_topic_in_criterias(publication_data.topicName(), rule.publishes))
            {
                if(rule.allow)
                {
                    returned_value = true;
                }
                else
                {
                    exception = _SecurityException_(publication_data.topicName() +
                            std::string(" topic denied by deny rule."));
                }

                break;
            }
        }
    }

    if(!returned_value && strlen(exception.what()) == 0)
    {
        exception = _SecurityException_(publication_data.topicName() +
                std::string(" topic not found in allow rule."));
    }

    return returned_value;
}

bool Permissions::check_remote_datareader(const PermissionsHandle& remote_handle,
        const uint32_t domain_id, const ReaderProxyData& subscription_data,
        SecurityException& exception)
{
    bool returned_value = false;
    const AccessPermissionsHandle& rah = AccessPermissionsHandle::narrow(remote_handle);

    if(rah.nil())
    {
        exception = _SecurityException_("Bad precondition");
        return false;
    }

    for(auto rule : rah->grant.rules)
    {
        if(is_domain_in_set(domain_id, rule.domains))
        {
            if(is_topic_in_criterias(subscription_data.topicName(), rule.subscribes))
            {
                if(rule.allow)
                {
                    returned_value = true;
                }
                else
                {
                    exception = _SecurityException_(subscription_data.topicName() +
                            std::string(" topic denied by deny rule."));
                }

                break;
            }
        }
    }

    if(!returned_value && strlen(exception.what()) == 0)
    {
        exception = _SecurityException_(subscription_data.topicName() +
                std::string(" topic not found in allow rule."));
    }

    return returned_value;
}
