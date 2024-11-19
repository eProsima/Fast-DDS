// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocalReaderPointer.hpp
 */

#ifndef FASTDDS_RTPS_READER__LOCALREADERPOINTER_HPP
#define FASTDDS_RTPS_READER__LOCALREADERPOINTER_HPP

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/utils/RefCountedPointer.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;

struct LocalReaderPointer : public RefCountedPointer<RTPSReader>
{
    LocalReaderPointer(
            RTPSReader* ptr)
        : RefCountedPointer<RTPSReader>(ptr)
    {
    }

    virtual ~LocalReaderPointer() = default;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FASTDDS_RTPS_READER__LOCALREADERPOINTER_HPP
