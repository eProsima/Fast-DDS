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
 * @file DServerEvent2.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DSERVEREVENT2_H_
#define _FASTDDS_RTPS_DSERVEREVENT2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/resources/TimedEvent.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPServer2;

// is keep the 2 in class name to remember that is new version
/**
 * Class DServerRoutineEvent2, TimedEvent used to synchronize discover-server servers
 *@ingroup DISCOVERY_MODULE
 */
class DServerRoutineEvent2 : public fastrtps::rtps::TimedEvent
{
public:

    /**
     * Constructor.
     * @param pdp Pointer to the PDPServer.
     * @param server_routine_period Interval in ms.
     */
    DServerRoutineEvent2(
            PDPServer2* pdp,
            double server_routine_period);
    ~DServerRoutineEvent2();

    /**
     * Method invoked when the server routine event occurs.
     */
    bool server_routine_event();

    //!Pointer to the PDPServer object.
    PDPServer2* pdp_;

    //! The period in milliseconds for the server to wait for ACK and execute the server routine.
    double server_routine_period_;
};

class DServerPingEvent2 : public fastrtps::rtps::TimedEvent
{
public:

    /**
     * Constructor.
     * @param pdp Pointer to the PDPServer.
     * @param interval Interval in ms.
     */
    DServerPingEvent2(
            PDPServer2* pdp,
            double interval);
    ~DServerPingEvent2();

    /**
     * Method invoked when the server routine event occurs.
     */
    bool server_ping_event();

    //!Pointer to the PDPServer object.
    PDPServer2* pdp_;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_DSERVEREVENT2_H_ */
