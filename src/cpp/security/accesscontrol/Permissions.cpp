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
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/security/exceptions/SecurityException.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

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

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::rtps::security;

static const char* const RSA_SHA256 = "RSASSA-PSS-SHA256";
static const char* const ECDSA_SHA256 = "ECDSA-SHA256";

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

static bool load_governance_file(AccessPermissionsHandle& ah, std::string& governance_file, SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, governance_file, exception);

    if(file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if(ptr != nullptr)
        {
            printf("%s", ptr->data);
            returned_value = true;
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

static bool load_permissions_file(AccessPermissionsHandle& ah, std::string& permissions_file, SecurityException& exception)
{
    bool returned_value = false;

    BIO* file_mem = load_signed_file(ah->store_, permissions_file, exception);

    if(file_mem != nullptr)
    {
        BUF_MEM* ptr = nullptr;
        BIO_get_mem_ptr(file_mem, &ptr);

        if(ptr != nullptr)
        {
            printf("%s", ptr->data);
            returned_value = true;
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

PermissionsHandle* Permissions::validate_local_permissions(Authentication& auth_plugin,
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
        if(load_governance_file(*ah, *governance, exception))
        {
            if(load_permissions_file(*ah, *permissions, exception))
            {
                return ah;
            }
        }
    }

    delete ah;

    return nullptr;
}
