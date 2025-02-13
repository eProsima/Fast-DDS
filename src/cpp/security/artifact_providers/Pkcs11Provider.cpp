// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Pkcs11Provider.cpp
 */

// TODO This isn't a proper fix for compatibility with OpenSSL 3.0, but
// suppresses the warnings until true OpenSSL 3.0 APIs can be used.
#ifdef OPENSSL_API_COMPAT
#undef OPENSSL_API_COMPAT
#endif // ifdef OPENSSL_API_COMPAT
#define OPENSSL_API_COMPAT 10101

#include <security/artifact_providers/Pkcs11Provider.hpp>

#include <iostream>

#include <fastdds/dds/log/Log.hpp>
#include <utils/SystemInfo.hpp>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION " (" __FILE__ ":" S2(__LINE__) ")"
#define _SecurityException_(str) SecurityException(std::string(str) + LOCATION)


namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {
namespace detail {

constexpr const char* FASTDDS_PKCS11_PIN = "FASTDDS_PKCS11_PIN";
constexpr const char* PKCS11_ENGINE_ID = "pkcs11";

static int ui_open(
        UI* ui)
{
    return UI_method_get_opener(UI_OpenSSL())(ui);
}

static int ui_read(
        UI* ui,
        UI_STRING* uis)
{
    switch (UI_get_string_type(uis))
    {
        case UIT_PROMPT:
        case UIT_VERIFY:
        {
            EPROSIMA_LOG_WARNING(PKCS11_PROVIDER, "PKCS#11 engine is asking: " << UI_get0_output_string(uis));
            // Return an empty password without asking the user
            UI_set_result(ui, uis, "");
            return 1;
        }
        default:
            break;
    }

    // Call the default method of the engine provider
    return UI_method_get_reader(UI_OpenSSL())(ui, uis);
}

static int ui_close(
        UI* ui)
{
    return UI_method_get_closer(UI_OpenSSL())(ui);
}

Pkcs11Provider::Pkcs11Provider()
{
    SSL_load_error_strings();                /* readable error messages */
    SSL_library_init();                      /* initialize library */

    // Create an UI method to use with the engine
    // This will be used to retrieve the PIN if none was given in the ENV nor in the URI
    ui_method_ = UI_create_method("OpenSSL application user interface");
    UI_method_set_opener(ui_method_, ui_open);
    UI_method_set_reader(ui_method_, ui_read);
    UI_method_set_closer(ui_method_, ui_close);

    // Load the engine
    ENGINE_load_builtin_engines();
    pkcs11_ = ENGINE_by_id(PKCS11_ENGINE_ID);
    if (!pkcs11_)
    {
        has_initialization_error_ = true;
        initialization_exception_ = _SecurityException_(std::string("Error retrieving 'pkcs11' engine"));
        return;
    }

    // Load the PIN from the environment
    std::string pin;
    if (fastdds::dds::RETCODE_OK == SystemInfo::get_env(FASTDDS_PKCS11_PIN, pin))
    {
        if (!ENGINE_ctrl_cmd_string( pkcs11_, "PIN", pin.c_str(), 0))
        {
            has_initialization_error_ = true;
            initialization_exception_ =
                    _SecurityException_(std::string("Error setting the PIN in the 'pkcs11' engine"));
            ENGINE_free(pkcs11_);
            return;
        }
    }

    // Init the engine with the PIN (if any)
    if (!ENGINE_init(pkcs11_))
    {
        has_initialization_error_ = true;
        initialization_exception_ = _SecurityException_(std::string("Error initializing the HSM provider library"));
        ENGINE_free(pkcs11_);
        return;
    }
}

Pkcs11Provider::~Pkcs11Provider()
{
    ENGINE_finish(pkcs11_);
    ENGINE_free(pkcs11_);

    if (ui_method_)
    {
        UI_destroy_method(ui_method_);
    }
}

EVP_PKEY* Pkcs11Provider::load_private_key(
        X509* certificate,
        const std::string& pkey,
        const std::string& /*password*/,
        SecurityException& exception)
{
    if (has_initialization_error_)
    {
        exception = initialization_exception_;
        return nullptr;
    }

    EVP_PKEY* returnedValue = ENGINE_load_private_key(pkcs11_, pkey.c_str(), ui_method_, nullptr);
    if (!returnedValue)
    {
        exception = _SecurityException_(std::string("Error opening the private key ") + pkey.substr(7));
        return returnedValue;
    }

    // Verify private key.
    if (!X509_check_private_key(certificate, returnedValue))
    {
        exception = _SecurityException_(std::string("Error verifying private key ") + pkey.substr(7)
                        + "\n ERROR: " + ERR_error_string(ERR_get_error(), nullptr));
        EVP_PKEY_free(returnedValue);
        returnedValue = nullptr;
    }

    return returnedValue;
}

} // namespace detail
} // namespace security
} // namespace rtps
} // namespace fastdds
} // namespace eprosima
