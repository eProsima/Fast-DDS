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

/**
 * @file ResendParticipantProxyDataPeriod.h
 *
 */

#ifndef RESENDDATAPERIOD_H_
#define RESENDDATAPERIOD_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PDPSimple;

/**
 * Class ResendParticipantProxyDataPeriod, TimedEvent used to periodically send the RTPSParticipantDiscovery Data.
 *@ingroup DISCOVERY_MODULE
 */
class ResendParticipantProxyDataPeriod: public TimedEvent
{
public:

    /**
     * Constructor.
     * @param p_SPDP Pointer to the PDPSimple.
     * @param config Configuration of builtin discovery.
     */
    ResendParticipantProxyDataPeriod(
            PDPSimple* p_SPDP,
            const BuiltinAttributes& config);

    virtual ~ResendParticipantProxyDataPeriod();
    
    /**
    * Method invoked when the event occurs.
    * This temporal event resends the RTPSParticipantProxyData to all remote RTPSParticipants.
    * @param code Code representing the status of the event
    * @param msg Message associated to the event
    */
    void event(
            EventCode code,
            const char* msg = nullptr) override;

private:

    void set_next_interval();
    
    //! Pointer to the PDPSimple object.
    PDPSimple* pdp_;
    //! Holds announcement period after the initial announcements have been sent
    Duration_t standard_period_;
    //! Holds number of initial announcements left and sending period
    InitialAnnouncementConfig initial_announcements_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif

#endif /* RESENDDATAPERIOD_H_ */
