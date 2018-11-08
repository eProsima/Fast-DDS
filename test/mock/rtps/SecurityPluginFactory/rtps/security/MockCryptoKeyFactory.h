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
 * @file MockCryptoKeyFactory.h
 */
#ifndef _RTPS_SECURITY_MOCKCRYPTOKEYFACTORY_H_
#define _RTPS_SECURITY_MOCKCRYPTOKEYFACTORY_H_

#include <fastrtps/rtps/security/cryptography/CryptoKeyFactory.h>
#include <fastrtps/rtps/security/cryptography/CryptoTypes.h>
#include <gmock/gmock.h>

#pragma warning(push)
#pragma warning(disable : 4373)

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class MockCryptoKeyFactory : public CryptoKeyFactory
{
    public:

        virtual ~MockCryptoKeyFactory(){}

        MOCK_METHOD5(register_local_participant, ParticipantCryptoHandle* (
                const IdentityHandle&,
                const PermissionsHandle&,
                const PropertySeq&,
                const ParticipantSecurityAttributes&,
                SecurityException&));

        MOCK_METHOD5(register_matched_remote_participant, ParticipantCryptoHandle* (
                const ParticipantCryptoHandle&,
                const IdentityHandle&,
                const PermissionsHandle&,
                const SharedSecretHandle&,
                SecurityException&));

        MOCK_METHOD4(register_local_datawriter, DatawriterCryptoHandle* (
                ParticipantCryptoHandle&,
                const PropertySeq&,
                const EndpointSecurityAttributes&,
                SecurityException&));

        MOCK_METHOD5(register_matched_remote_datareader, DatareaderCryptoHandle* (
                DatawriterCryptoHandle&,
                ParticipantCryptoHandle&,
                const SharedSecretHandle&,
                const bool,
                SecurityException&));

        MOCK_METHOD4(register_local_datareader, DatareaderCryptoHandle* (
                ParticipantCryptoHandle&,
                const PropertySeq&,
                const EndpointSecurityAttributes&,
                SecurityException&));

        MOCK_METHOD4(register_matched_remote_datawriter, DatawriterCryptoHandle* (
                DatareaderCryptoHandle&,
                ParticipantCryptoHandle&,
                const SharedSecretHandle&,
                SecurityException&));

        MOCK_METHOD2(unregister_participant, bool (
                ParticipantCryptoHandle*,
                SecurityException&));

        MOCK_METHOD2(unregister_datawriter, bool (
                DatawriterCryptoHandle*,
                SecurityException&));

        MOCK_METHOD2(unregister_datareader, bool (
                DatareaderCryptoHandle*,
                SecurityException&));


};

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#pragma warning(pop)

#endif // _RTPS_SECURITY_MOCKCRYPTOKEYFACTORY_H_
