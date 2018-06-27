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

#include <fastrtps/types/MemberDescriptor.h>

namespace eprosima {
namespace fastrtps {
namespace types {

MemberDescriptor::MemberDescriptor()
: mIndex(0)
{
}

ResponseCode MemberDescriptor::copy_from(const MemberDescriptor*) const
{
    return ResponseCode::RETCODE_OK;
}

bool MemberDescriptor::equals(const MemberDescriptor&) const
{
    return false;
}
bool MemberDescriptor::isConsistent() const
{
    return false;
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
