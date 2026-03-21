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
 * @file AESGCMGMAC_Types.h
 */

#ifndef _SECURITY_AUTHENTICATION_AESGCMGMAC_TYPES_H_
#define _SECURITY_AUTHENTICATION_AESGCMGMAC_TYPES_H_

#include <cassert>
#include <functional>
#include <limits>
#include <mutex>

#include <fastdds/rtps/attributes/PropertyPolicy.hpp>
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>

#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#include <rtps/security/common/Handle.h>
#include <rtps/security/common/SharedSecretHandle.h>
#include <rtps/security/exceptions/SecurityException.h>
#include <rtps/security/cryptography/CryptoTypes.h>

// Fix compilation error on Windows
#if defined(WIN32) && defined(max)
#undef max
#endif // if defined(WIN32) && defined(max)

//No encryption, no authentication tag
#define CRYPTO_TRANSFORMATION_KIND_NONE             { {0, 0, 0, 0} }

//No encryption, AES128-GMAC authentication
#define CRYPTO_TRANSFORMATION_KIND_AES128_GMAC      { {0, 0, 0, 1} }

//Authenticated encryption via AES128
#define CRYPTO_TRANSFORMATION_KIND_AES128_GCM       { {0, 0, 0, 2} }

//No encryption, AES256-GMAC authentication
#define CRYPTO_TRANSFORMATION_KIND_AES256_GMAC      { {0, 0, 0, 3} }

// Authenticated encryption via AES256-GMC
#define CRYPTO_TRANSFORMATION_KIND_AES256_GCM       { {0, 0, 0, 4} }

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

constexpr CryptoTransformKind c_transfrom_kind_none = CRYPTO_TRANSFORMATION_KIND_NONE;
constexpr CryptoTransformKind c_transfrom_kind_aes128_gmac = CRYPTO_TRANSFORMATION_KIND_AES128_GMAC;
constexpr CryptoTransformKind c_transfrom_kind_aes128_gcm = CRYPTO_TRANSFORMATION_KIND_AES128_GCM;
constexpr CryptoTransformKind c_transfrom_kind_aes256_gmac = CRYPTO_TRANSFORMATION_KIND_AES256_GMAC;
constexpr CryptoTransformKind c_transfrom_kind_aes256_gcm = CRYPTO_TRANSFORMATION_KIND_AES256_GCM;

constexpr std::array<uint8_t, 32> c_empty_key_material =
{ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

/* Key Storage
 * -----------
 *  Contains common key and specific key
 *      -The common key is used to cipher (same for all receivers)
 *      -The specific key is used to sign (specific for each receiver)
 *  One KeyMaterial is used to store either:
 *      -The keys needed to send a message to another element
 *      -The keys needed to receive a message from another element
 *  Note: Key_Ids are ensured to be unique within a Cryptogaphic domain (Participant)
 */
struct KeyMaterial_AES_GCM_GMAC
{
    CryptoTransformKind transformation_kind = c_transfrom_kind_none;
    std::array<uint8_t, 32> master_salt = c_empty_key_material;

    CryptoTransformKeyId sender_key_id = c_transfrom_kind_none;
    std::array<uint8_t, 32> master_sender_key = c_empty_key_material;

    CryptoTransformKeyId receiver_specific_key_id = c_transfrom_kind_none;
    std::array<uint8_t, 32> master_receiver_specific_key = c_empty_key_material;
};

typedef std::vector<KeyMaterial_AES_GCM_GMAC> KeyMaterial_AES_GCM_GMAC_Seq;

/* SecureSubMessageElements
 * ------------------------
 */

//Holds information about the type of encryption performed, the id of the key to use
//and the initialization vector
struct SecureDataHeader
{
    CryptoTransformIdentifier transform_identifier;
    std::array<uint8_t, 4> session_id;
    std::array<uint8_t, 8> initialization_vector_suffix;
};

//Holds the ciphered data
struct SecureDataBody
{
    std::vector<uint8_t> secure_data;
};

//Holds signatures.
//common_mac->Signature using the common key that every intended receiver had
//specific_mac->SignatureS made with the specific keys that only each pair of sender/receiver knows
struct SecureDataTag
{
    std::array<uint8_t, 16> common_mac;
    CryptoTransformKeyId receiver_mac_key_id;
    std::array<uint8_t, 16> receiver_mac;
};

/* Key Management
 * --------------
 * Keys are stored and managed as Cryptohandles
 * There are CryptoHandles for Participant, DataWriter and DataReader keys
 * Each CryptoHandle stores different data, but share common traits.
 *
 * All CryptoHandle instances hold
 * -A copy of the common key: the key used to cypher and known by all possible receivers
 * -A copy of the (direct) specific key: the key used to sign outgoing messages (receiver_specific_macs).
 * -A copy of the (reverse) specific key: the key used to verify the signature of incoming messages.
 *
 * In the case of a LocalCryptoHandle, one instance of the specific keys is stored for each matching element.
 * In the case of a RemoteCryptoHandle, only the keys pertaining the remote element are stored.
 *
 * Note: the common key of the remote cryptohandle is stored along with the specific keys. KeyMaterial->master_sender_key
 */

struct KeySessionData
{
    uint32_t session_id = (std::numeric_limits<uint32_t>::max)();
    std::array<uint8_t, 32> SessionKey = c_empty_key_material;
    uint64_t session_block_counter = 0;
};

struct EntityKeyHandle
{
    static const char* const class_id_;

    // Reference to an auxiliary exception object on destruction
    SecurityException* exception_ = {nullptr};

    //Plugin security options
    PluginEndpointSecurityAttributesMask EndpointPluginAttributes = 0;
    //Storage for the LocalCryptoHandle master_key, not used in RemoteCryptoHandles
    KeyMaterial_AES_GCM_GMAC_Seq EntityKeyMaterial;
    //KeyId of the master_key of the parent Participant and pointer to the relevant CryptoHandle
    CryptoTransformKeyId Participant_master_key_id = c_transformKeyIdZero;
    std::weak_ptr<ParticipantCryptoHandle> Parent_participant;

    //(Direct) ReceiverSpecific Keys - Inherently hold the master_key of the writer
    KeyMaterial_AES_GCM_GMAC_Seq Entity2RemoteKeyMaterial;
    //(Reverse) ReceiverSpecific Keys - Inherently hold the master_key of the remote readers
    KeyMaterial_AES_GCM_GMAC_Seq Remote2EntityKeyMaterial;
    //Copy of the Keymaterial used to Cypher CryptoTokens (inherited from the parent participant)
    // KeyMaterial_AES_GCM_GMAC Participant2ParticipantKxKeyMaterial;

    //Data used to store the current session keys and to determine when it has to be updated
    KeySessionData Sessions[2];
    uint64_t max_blocks_per_session = 0;
    std::mutex mutex_;
};

class AESGCMGMAC_KeyFactory;

typedef HandleImpl<EntityKeyHandle, AESGCMGMAC_KeyFactory> AESGCMGMAC_WriterCryptoHandle;
typedef HandleImpl<EntityKeyHandle, AESGCMGMAC_KeyFactory> AESGCMGMAC_ReaderCryptoHandle;
typedef HandleImpl<EntityKeyHandle, AESGCMGMAC_KeyFactory> AESGCMGMAC_EntityCryptoHandle;

struct ParticipantKeyHandle
{
    static const char* const class_id_;

    // Reference to an auxiliary exception object on destruction
    SecurityException* exception_ = {nullptr};

    //Plugin security options
    PluginParticipantSecurityAttributesMask ParticipantPluginAttributes = 0;
    //Storage for the LocalCryptoHandle master_key, not used in RemoteCryptoHandles
    KeyMaterial_AES_GCM_GMAC ParticipantKeyMaterial;
    //(Direct) ReceiverSpecific Keys - Inherently hold the master_key of the writer
    std::vector<KeyMaterial_AES_GCM_GMAC> Participant2ParticipantKeyMaterial;
    //Keymaterial used to Cypher CryptoTokens (inherited from the parent participant)
    std::vector<KeyMaterial_AES_GCM_GMAC> Participant2ParticipantKxKeyMaterial;
    //(Reverse) ReceiverSpecific Keys - Inherently hold the master_key of the remote readers
    std::vector<KeyMaterial_AES_GCM_GMAC> RemoteParticipant2ParticipantKeyMaterial;
    //List of Pointers to the CryptoHandles of all matched Writers
    std::vector<std::shared_ptr<DatawriterCryptoHandle>> Writers;
    //List of Pointers to the CryptoHandles of all matched Readers
    std::vector<std::shared_ptr<DatareaderCryptoHandle>> Readers;

    //Data used to store the current session keys and to determine when it has to be updated
    KeySessionData Session;
    uint64_t max_blocks_per_session = {0};
    std::mutex mutex_;
};

typedef HandleImpl<ParticipantKeyHandle, AESGCMGMAC_KeyFactory> AESGCMGMAC_ParticipantCryptoHandle;

} //namespaces security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // _SECURITY_AUTHENTICATION_AESGCMGMAC_TYPES_H_
