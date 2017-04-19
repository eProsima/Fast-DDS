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
 * @file mockAccessHandle.h
 */
#ifndef _SECURITY_ACCESS_MOCKACCESSHANDLE_H_
#define _SECURITY_ACCESS_MOCKACCESSHANDLE_H_

#include <fastrtps/rtps/security/common/Handle.h>
#include <fastrtps/rtps/common/Guid.h>
#include <string>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace security {

class mockAccess
{
    public:

        mockAccess(){}

        ~mockAccess(){}

        static const char* const class_id_;

};

typedef HandleImpl<mockAccess> mockAccessHandle;

} //namespace security
} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _SECURITY_ACESS_MOCKACCESSHANDLE_H_
