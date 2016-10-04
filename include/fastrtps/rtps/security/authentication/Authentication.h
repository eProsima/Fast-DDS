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
 * @file Authentication.h	
 */
#ifndef _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_
#define _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_

#include "../common/Handle.h"
#include "../../common/Guid.h"
#include "../../attributes/RTPSParticipantAttributes.h"
#include "../exceptions/SecurityException.h"

#include <cstdint>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

enum ValidationResult_t : uint32_t
{
    VALIDATION_OK = 0,
    VALIDATION_FAILED,
    VALIDATION_PENDING_RETRY,
    VALIDATION_PENDING_HANDSHAKE_REQUEST,
    VALIDATION_PENDING_HANDSHAKE_MESSAGE,
    VALIDATION_OK_FINAL_MESSAGE
};

class Authentication
{
    public:

        /*!
         * @brief Validates the identity of the local RTPSParticipant.
         * @param local_identity_handle (out) A handle that can be used to locally refer to the Authenticated
         * Participant in subsequent interactions with the Authentication plugin.
         * @param adjusted_participant_key (out) The GUID_t that the implementation shall use to uniquely identify the
         * RTPSParticipant on the network.
         * @param domain_id The Domain Id of the RTPSParticipant.
         * @param participant_attr The RTPSParticipantAttributes of the RTPSParticipant.
         * @param candidate_participant_key The GUID_t that the DDS implementation would have used to uniquely identify
         * the RTPSParticipant if the Security plugins were not enabled.
         */
        virtual ValidationResult_t validate_local_identity(IdentityHandle** local_identity_handle,
                GUID_t& adjusted_participant_key,
                const uint32_t domain_id,
                const RTPSParticipantAttributes& participant_attr,
                const GUID_t& candidate_participant_key,
                SecurityException& exception) = 0;
};

} //namespace eprosima
} //namespace fastrtps
} //namespace rtps
} //namespace security

#endif //  _RTPS_SECURITY_AUTHENTICATION_AUTHENTICATION_H_
