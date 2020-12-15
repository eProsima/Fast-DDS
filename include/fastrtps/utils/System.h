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

/**
 * @file System.h
 *
 */

#ifndef _EPROSIMA_SYSTEM_UTILS_H
#define _EPROSIMA_SYSTEM_UTILS_H

#include "../fastrtps_dll.h"

namespace eprosima {
namespace fastrtps {

/**
 * Class System, to provide helper functions to access system information.
 * @ingroup UTILITIES_MODULE
 */
class System
{
public:

    //! Returns current process identifier.
    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::System::GetPID", "")
    RTPS_DllAPI static int GetPID();
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _EPROSIMA_SYSTEM_UTILS_H */
