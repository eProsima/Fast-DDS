// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LicenseTools.hpp
 */

#ifndef UTILS_LICENSE__LICENSETOOLS_HPP_
#define UTILS_LICENSE__LICENSETOOLS_HPP_

#if HAVE_SECURITY
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#endif  // HAVE_SECURITY

#include <fastdds/rtps/common/Types.h>

namespace eprosima {

using fastrtps::rtps::octet;

/**
 * @brief Verify signature of data using eProsima's licensing public key.
 *
 * @param data Pointer to the data to verify.
 * @param data_length Length of the data to verify.
 * @param signature Pointer to the signature.
 * @param signature_length Length of the signature.
 *
 * @return true if the signature is valid, false otherwise.
 */
bool verify_safedds_signature(
        const octet* data,
        size_t data_length,
        const octet* signature,
        size_t signature_length)
{
#if HAVE_SECURITY
    static const unsigned char pubkey_der[] =
    {
        0x30, 0x2a, 0x30, 0x05, 0x06, 0x03, 0x2b, 0x65, 0x70, 0x03, 0x21, 0x00, 0x4a, 0x01, 0xe9, 0xd4,
        0x16, 0x79, 0xbd, 0x2e, 0x17, 0xeb, 0xc9, 0x68, 0x07, 0xd7, 0x65, 0x82, 0x0d, 0x56, 0x5d, 0x61,
        0x8c, 0xab, 0x77, 0xdb, 0x55, 0xd2, 0xa1, 0x2e, 0x31, 0xaa, 0xdb, 0xaa
    };

    struct OperationCTX
    {
        EVP_PKEY* pkey = nullptr;
        EVP_MD_CTX* md_ctx = nullptr;

        ~OperationCTX()
        {
            if (md_ctx != nullptr)
            {
                EVP_MD_CTX_free(md_ctx);
            }
            if (pkey != nullptr)
            {
                EVP_PKEY_free(pkey);
            }
        }

    };

    OperationCTX ctx;

    const unsigned char* p = pubkey_der;
    ctx.pkey = d2i_PUBKEY(nullptr, &p, sizeof(pubkey_der));
    if (ctx.pkey == nullptr)
    {
        return false;
    }

    ctx.md_ctx = EVP_MD_CTX_new();
    if (ctx.md_ctx == nullptr)
    {
        return false;
    }

    if (EVP_DigestVerifyInit(ctx.md_ctx, nullptr, nullptr, nullptr, ctx.pkey) != 1)
    {
        return false;
    }

    int verify_result = EVP_DigestVerify(ctx.md_ctx, signature, signature_length, data, data_length);
    return verify_result == 1;
#else
    static_cast<void>(data);
    static_cast<void>(data_length);
    static_cast<void>(signature);
    static_cast<void>(signature_length);

    return true;
#endif  // HAVE_SECURITY
}

}  // namespace eprosima

#endif  // UTILS_LICENSE__LICENSETOOLS_HPP_
