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
 * @file CryptoTypes.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOTYPES_H_
#define _FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOTYPES_H_

#include <fastdds/rtps/common/Token.hpp>
#include <rtps/security/common/Handle.h>
#include <rtps/security/common/SharedSecretHandle.h>
#include <rtps/security/exceptions/SecurityException.h>
#include <array>

#define GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS "dds.sec.participant_crypto_tokens"
#define GMCLASSID_SECURITY_DATAWRITER_CRYPTO_TOKENS "dds.sec.datawriter_crypto_tokens"
#define GMCLASSID_SECURITY_DATAREADER_CRYPTO_TOKENS "dds.sec.datareader_crypto_tokens"

#define _SEC_PREFIX_              0x31
#define _SEC_POSTFIX_             0x32
#define _SRTPS_PREFIX_            0x33
#define _SRTPS_POSTFIX_           0x34
#define _SecureBodySubmessage_    0x30

const uint8_t SEC_PREFIX = _SEC_PREFIX_;
const uint8_t SEC_POSTFIX = _SEC_POSTFIX_;
const uint8_t SRTPS_PREFIX = _SRTPS_PREFIX_;
const uint8_t SRTPS_POSTFIX = _SRTPS_POSTFIX_;
const uint8_t SecureBodySubmessage = _SecureBodySubmessage_;

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

typedef std::array<uint8_t, 4> CryptoTransformKind;
typedef std::array<uint8_t, 4> CryptoTransformKeyId;

constexpr CryptoTransformKeyId c_transformKeyIdZero = { {0, 0, 0, 0} };

typedef Token MessageToken;
typedef MessageToken AuthRequestMessageToken;
typedef MessageToken HandshakeMessageToken;

typedef Token CryptoToken;
typedef Token ParticipantCryptoToken;
typedef Token DatawriterCryptoToken;
typedef Token DatareaderCryptoToken;

typedef std::vector<HandshakeMessageToken> HandshakeMessageTokenSeq;
typedef std::vector<CryptoToken> CryptoTokenSeq;
typedef CryptoTokenSeq ParticipantCryptoTokenSeq;
typedef CryptoTokenSeq DatawriterCryptoTokenSeq;
typedef CryptoTokenSeq DatareaderCryptoTokenSeq;

struct CryptoTransformIdentifier
{
    CryptoTransformKind transformation_kind;
    CryptoTransformKeyId transformation_key_id;
};

enum SecureSubmessageCategory_t : uint8_t
{
    INFO_SUBMESSAGE = 0,
    DATAWRITER_SUBMESSAGE,
    DATAREADER_SUBMESSAGE
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif //_FASTDDS_RTPS_SECURITY_CRYPTOGRAPHY_CRYPTOTYPES_H_
