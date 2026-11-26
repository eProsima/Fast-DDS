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
#ifndef FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYFACTORY_H
#define FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYFACTORY_H

#include <gmock/gmock.h>

#include <rtps/security/cryptography/CryptoKeyFactory.h>
#include <rtps/security/cryptography/CryptoTypes.h>

#include <security/cryptography/AESGCMGMAC_Types.h>

#pragma warning(push)
#pragma warning(disable : 4373)

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class MockCryptoKeyFactory : public CryptoKeyFactory
{
public:

    using AESGCMGMAC_ParticipantCryptoHandle = HandleImpl<ParticipantKeyHandle, MockCryptoKeyFactory>;

    virtual ~MockCryptoKeyFactory()
    {
    }

    MOCK_METHOD5(register_local_participant, std::shared_ptr<ParticipantCryptoHandle> (
                const IdentityHandle&,
                const PermissionsHandle&,
                const PropertySeq&,
                const ParticipantSecurityAttributes&,
                SecurityException&));

    MOCK_METHOD5(register_matched_remote_participant, std::shared_ptr<ParticipantCryptoHandle> (
                const ParticipantCryptoHandle&,
                const IdentityHandle&,
                const PermissionsHandle&,
                const SecretHandle&,
                SecurityException&));

    MOCK_METHOD4(register_local_datawriter, DatawriterCryptoHandle * (
                ParticipantCryptoHandle&,
                const PropertySeq&,
                const EndpointSecurityAttributes&,
                SecurityException &));

    MOCK_METHOD5(register_matched_remote_datareader, DatareaderCryptoHandle * (
                DatawriterCryptoHandle&,
                ParticipantCryptoHandle&,
                const SecretHandle&,
                const bool,
                SecurityException &));

    MOCK_METHOD4(register_local_datareader, DatareaderCryptoHandle * (
                ParticipantCryptoHandle&,
                const PropertySeq&,
                const EndpointSecurityAttributes&,
                SecurityException &));

    MOCK_METHOD4(register_matched_remote_datawriter, DatawriterCryptoHandle * (
                DatareaderCryptoHandle&,
                ParticipantCryptoHandle&,
                const SecretHandle&,
                SecurityException &));

    MOCK_METHOD2(unregister_participant, bool (
                std::shared_ptr<ParticipantCryptoHandle>&,
                SecurityException &));

    MOCK_METHOD2(unregister_datawriter, bool (
                DatawriterCryptoHandle*,
                SecurityException &));

    MOCK_METHOD2(unregister_datareader, bool (
                DatareaderCryptoHandle*,
                SecurityException &));

    MOCK_METHOD2(unregister_datawriter, bool (
                std::shared_ptr<DatawriterCryptoHandle>&,
                SecurityException &));

    MOCK_METHOD2(unregister_datareader, bool (
                std::shared_ptr<DatareaderCryptoHandle>&,
                SecurityException &));

    std::shared_ptr<ParticipantCryptoHandle> get_dummy_participant_handle() const
    {
        // create ad hoc deleter because this object can only be created/release from the friend factory
        auto p = new (std::nothrow) AESGCMGMAC_ParticipantCryptoHandle;
        return std::dynamic_pointer_cast<ParticipantCryptoHandle>(
            std::shared_ptr<AESGCMGMAC_ParticipantCryptoHandle>(p,
            [](AESGCMGMAC_ParticipantCryptoHandle* p)
            {
                delete p;
            }));
    }

};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#pragma warning(pop)

#endif // FASTDDS_RTPS_SECURITY__MOCKCRYPTOKEYFACTORY_H
