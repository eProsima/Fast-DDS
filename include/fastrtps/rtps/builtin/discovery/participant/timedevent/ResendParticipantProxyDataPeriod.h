/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResendParticipantProxyDataPeriod.h
 *
 */

#ifndef RESENDDATAPERIOD_H_
#define RESENDDATAPERIOD_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "fastrtps/rtps/resources/TimedEvent.h"
#include "fastrtps/rtps/common/CDRMessage_t.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class PDPSimple;

/**
 * Class ResendParticipantProxyDataPeriod, TimedEvent used to periodically send the RTPSParticipantDiscovery Data.
 *@ingroup DISCOVERY_MODULE
 */
class ResendParticipantProxyDataPeriod: public TimedEvent {
public:

	/**
	 * Constructor.
	 * @param p_SPDP Pointer to the PDPSimple.
	 * @param interval Interval in ms.
	 */
	ResendParticipantProxyDataPeriod(PDPSimple* p_SPDP,
			double interval);
	virtual ~ResendParticipantProxyDataPeriod();
	
	/**
	* Method invoked when the event occurs.
	* This temporal event resends the RTPSParticipantProxyData to all remote RTPSParticipants.
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);
	
	//!Auxiliar data message.
	CDRMessage_t m_data_msg;
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RESENDDATAPERIOD_H_ */
