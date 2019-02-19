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

#ifndef _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
#define _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_

#include <gmock/gmock.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Forward declarations
class StatefulReader;
struct GUID_t;

class WriterProxyLiveliness
{
    public:

        WriterProxyLiveliness(
                StatefulReader* /*reader*/,
                const GUID_t& /*guid*/,
                double /*interval*/)
        {
        }

        MOCK_METHOD0(restart_timer, void());
        MOCK_METHOD0(cancel_timer, void());
};

} //namespace rtps
} //namespace fastrtps
} //namespace eprosima

#endif // _RTPS_READER_TIMEDEVENT_WRITERPROXYLIVELINESS_H_
