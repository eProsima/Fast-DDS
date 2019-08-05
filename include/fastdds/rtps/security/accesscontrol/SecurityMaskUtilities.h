// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SecurityMaskUtilities.h
 */
#ifndef _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_SECURITYMASKUTILITIES_H_
#define _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_SECURITYMASKUTILITIES_H_

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

inline bool security_mask_matches(const uint32_t lv, const uint32_t rv)
{
    if (((lv & (1UL << 31)) == 0) || ((rv & (1UL << 31)) == 0))
    {
        return true;
    }

    return lv == rv;
}

}
}
}
}

#endif // _FASTDDS_RTPS_SECURITY_ACCESSCONTROL_SECURITYMASKUTILITIES_H_
