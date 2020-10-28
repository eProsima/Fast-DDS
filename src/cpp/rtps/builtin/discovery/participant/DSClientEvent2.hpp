// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DSClientEvent2.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DSCLIENTEVENT2_H_
#define _FASTDDS_RTPS_DSCLIENTEVENT2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/resources/TimedEvent.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPClient2;

/**
 * Class DSClientEvent2, TimedEvent used to synchronize discover-server clients
 *@ingroup DISCOVERY_MODULE
 */
class DSClientEvent2 : public fastrtps::rtps::TimedEvent
{
public:

    /**
     * Constructor.
     * @param p_PDP Pointer to the PDPClient.
     * @param interval Interval in ms.
     */
    DSClientEvent2(
            PDPClient2* p_PDP,
            double interval);
    ~DSClientEvent2();

    /**
     * Method invoked when the event occurs.
     */
    bool event();

    //!Pointer to the PDPClient2 object.
    PDPClient2* mp_PDP;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_DSCLIENTEVENT2_H_ */
