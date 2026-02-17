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
 * @file MockCryptoTransform.h
 */
#ifndef FASTDDS_RTPS_SECURITY__MOCKCRYPTOTRANSFORM_H
#define FASTDDS_RTPS_SECURITY__MOCKCRYPTOTRANSFORM_H

#include <gmock/gmock.h>

#include <rtps/security/cryptography/CryptoTransform.h>
#include <rtps/security/cryptography/CryptoTypes.h>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace security {

class MockCryptoTransform : public CryptoTransform
{
public:

    virtual ~MockCryptoTransform()
    {
    }

    MOCK_METHOD5(encode_serialized_payload, bool (
                SerializedPayload_t&,
                std::vector<uint8_t>&,
                const SerializedPayload_t&,
                DatawriterCryptoHandle&,
                SecurityException &));

    MOCK_METHOD5(encode_datawriter_submessage, bool (
                CDRMessage_t&,
                const CDRMessage_t&,
                DatawriterCryptoHandle&,
                std::vector<std::shared_ptr<DatareaderCryptoHandle>>&,
                SecurityException &));

    MOCK_METHOD5(encode_datareader_submessage, bool (
                CDRMessage_t&,
                const CDRMessage_t&,
                DatareaderCryptoHandle&,
                std::vector<std::shared_ptr<DatawriterCryptoHandle>>&,
                SecurityException & exception));

    MOCK_METHOD5(encode_rtps_message, bool (
                CDRMessage_t&,
                const CDRMessage_t&,
                ParticipantCryptoHandle&,
                std::vector<std::shared_ptr<ParticipantCryptoHandle>>&,
                SecurityException &));

    MOCK_METHOD5(decode_rtps_message, bool (
                CDRMessage_t&,
                const CDRMessage_t&,
                const ParticipantCryptoHandle&,
                const ParticipantCryptoHandle&,
                SecurityException &));

    MOCK_METHOD7(preprocess_secure_submsg, bool (
                DatawriterCryptoHandle * *,
                DatareaderCryptoHandle * *,
                SecureSubmessageCategory_t&,
                const CDRMessage_t&,
                ParticipantCryptoHandle&,
                ParticipantCryptoHandle&,
                SecurityException &));

    MOCK_METHOD5(decode_datawriter_submessage, bool (
                CDRMessage_t&,
                CDRMessage_t&,
                DatareaderCryptoHandle&,
                DatawriterCryptoHandle&,
                SecurityException &));

    MOCK_METHOD5(decode_datareader_submessage, bool (
                CDRMessage_t&,
                CDRMessage_t&,
                DatawriterCryptoHandle&,
                DatareaderCryptoHandle&,
                SecurityException &));

    MOCK_METHOD6(decode_serialized_payload, bool (
                SerializedPayload_t&,
                const SerializedPayload_t&,
                const std::vector<uint8_t>&,
                DatareaderCryptoHandle&,
                DatawriterCryptoHandle&,
                SecurityException &));

    uint32_t calculate_extra_size_for_rtps_message(
            uint32_t /*number_discovered_participants*/) const
    {
        return 0;
    }

    uint32_t calculate_extra_size_for_rtps_submessage(
            uint32_t /*number_discovered_readers*/) const
    {
        return 0;
    }

    uint32_t calculate_extra_size_for_encoded_payload(
            uint32_t /*number_discovered_readers*/) const
    {
        return 0;
    }

};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif //FASTDDS_RTPS_SECURITY__MOCKCRYPTOTRANSFORM_H
