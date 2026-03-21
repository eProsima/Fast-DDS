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
 * @file MockCryptoKeyExchange.h
 */
#ifndef FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYEXCHANGE_H
#define FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYEXCHANGE_H

#include <gmock/gmock.h>

#include <rtps/security/cryptography/CryptoKeyExchange.h>
#include <rtps/security/cryptography/CryptoTypes.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class MockCryptoKeyExchange : public CryptoKeyExchange
{
public:

    virtual ~MockCryptoKeyExchange()
    {
    }

    MOCK_METHOD4(create_local_participant_crypto_tokens, bool (
                ParticipantCryptoTokenSeq&,
                const ParticipantCryptoHandle&,
                ParticipantCryptoHandle&,
                SecurityException &));

    MOCK_METHOD4(set_remote_participant_crypto_tokens, bool (
                const ParticipantCryptoHandle&,
                ParticipantCryptoHandle&,
                const ParticipantCryptoTokenSeq&,
                SecurityException &));

    MOCK_METHOD4(create_local_datawriter_crypto_tokens, bool (
                DatawriterCryptoTokenSeq&,
                DatawriterCryptoHandle&,
                DatareaderCryptoHandle&,
                SecurityException &));

    MOCK_METHOD4(create_local_datareader_crypto_tokens, bool (
                DatareaderCryptoTokenSeq&,
                DatareaderCryptoHandle&,
                DatawriterCryptoHandle&,
                SecurityException &));

    MOCK_METHOD4(set_remote_datareader_crypto_tokens, bool (
                DatawriterCryptoHandle&,
                DatareaderCryptoHandle&,
                const DatareaderCryptoTokenSeq&,
                SecurityException &));

    MOCK_METHOD4(set_remote_datawriter_crypto_tokens, bool (
                DatareaderCryptoHandle&,
                DatawriterCryptoHandle&,
                const DatawriterCryptoTokenSeq&,
                SecurityException &));

    MOCK_METHOD2(return_crypto_tokens, bool (
                const CryptoTokenSeq&,
                SecurityException &));

};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif //FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYEXCHANGE_H
