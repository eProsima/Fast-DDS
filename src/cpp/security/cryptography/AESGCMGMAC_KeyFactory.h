// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file AESGCMGMAC_KeyFactory.h
 */

#ifndef _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYFACTORY_H_
#define _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYFACTORY_H_


#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <rtps/security/cryptography/CryptoKeyFactory.h>
#include <security/cryptography/AESGCMGMAC_Types.h>

#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class AESGCMGMAC_KeyFactory;

namespace {

// Auxiliary class to delete the handles, privy to this compilation unit
struct ParticipantCryptoHandleDeleter
{
    ParticipantCryptoHandleDeleter(
            AESGCMGMAC_KeyFactory& factory);

    void operator ()(
            AESGCMGMAC_ParticipantCryptoHandle* pk);

    // Reference to the factory
    std::weak_ptr<AESGCMGMAC_KeyFactory> factory_;
};

} // Unnamed namespace

class AESGCMGMAC_KeyFactory : public CryptoKeyFactory, public std::enable_shared_from_this<AESGCMGMAC_KeyFactory>
{
    friend ParticipantCryptoHandleDeleter;

    // Actual unregister_participant() implementation called from the handle destructor
    void release_participant(
            AESGCMGMAC_ParticipantCryptoHandle& key);

    // DatawriterCryptoHandle & DatareaderCryptoHandle creation methods.
    std::shared_ptr<DatawriterCryptoHandle> get_datawriter_handle();
    std::shared_ptr<DatareaderCryptoHandle> get_datareader_handle();

public:

    AESGCMGMAC_KeyFactory();

    std::shared_ptr<ParticipantCryptoHandle> register_local_participant(
            const IdentityHandle& participant_identity,
            const PermissionsHandle& participant_permissions,
            const PropertySeq& participant_properties,
            const ParticipantSecurityAttributes& participant_security_attributes,
            SecurityException& exception) override;

    std::shared_ptr<ParticipantCryptoHandle> register_matched_remote_participant(
            const ParticipantCryptoHandle& local_participant_crypto_handle,
            const IdentityHandle& remote_participant_identity,
            const PermissionsHandle& remote_participant_permissions,
            const SecretHandle& shared_secret,
            SecurityException& exception) override;

    DatawriterCryptoHandle* register_local_datawriter(
            ParticipantCryptoHandle& participant_crypto,
            const PropertySeq& datawriter_prop,
            const EndpointSecurityAttributes& datawriter_security_properties,
            SecurityException& exception) override;

    DatareaderCryptoHandle* register_matched_remote_datareader(
            DatawriterCryptoHandle& local_datawriter_crypto_handle,
            ParticipantCryptoHandle& remote_participant_crypto,
            const SecretHandle& shared_secret,
            const bool relay_only,
            SecurityException& exception) override;

    DatareaderCryptoHandle* register_local_datareader(
            ParticipantCryptoHandle& participant_crypto,
            const PropertySeq& datareader_properties,
            const EndpointSecurityAttributes& datareader_security_properties,
            SecurityException& exception) override;

    DatawriterCryptoHandle* register_matched_remote_datawriter(
            DatareaderCryptoHandle& local_datareader_crypto_handle,
            ParticipantCryptoHandle& remote_participant_crypt,
            const SecretHandle& shared_secret,
            SecurityException& exception) override;

    bool unregister_participant(
            std::shared_ptr<ParticipantCryptoHandle>& participant_crypto_handle,
            SecurityException& exception) override;

    bool unregister_datawriter(
            std::shared_ptr<DatawriterCryptoHandle>& datawriter_crypto_handle,
            SecurityException& exception) override;

    bool unregister_datareader(
            std::shared_ptr<DatareaderCryptoHandle>& datareader_crypto_handle,
            SecurityException& exception) override;

    // introduce convenient overrides in this scope
    using CryptoKeyFactory::unregister_datawriter;
    using CryptoKeyFactory::unregister_datareader;

private:

    /*
     * Create a new key material without receiver specific key
     */
    void create_key(
            KeyMaterial_AES_GCM_GMAC& key,
            bool encrypt_then_sign,
            bool use_256_bits);

    /*
     *  make_unique_KeyId();
     *  Generates an unique, unused CryptoTransformKeyId within the cryptographic domain
     *  Use this method to generate KeyIds
     */
    CryptoTransformKeyId make_unique_KeyId();

    void release_key_id(
            CryptoTransformKeyId id);

    //Storage for KeyIds in use
    std::vector<CryptoTransformKeyId> m_CryptoTransformKeyIds;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_KEYFACTORY_H_
