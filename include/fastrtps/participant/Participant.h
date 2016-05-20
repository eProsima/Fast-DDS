/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.h
 *
 */

#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#include "../rtps/common/Guid.h"
#include "../rtps/filters/FlowFilter.h"

#include "../rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{


class ParticipantImpl;
class ParticipantAttributes;

/**
 * Class Participant used to group Publishers and Subscribers into a single working unit.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI Participant {
	friend class Domain;
	friend class ParticipantImpl;
private:
	Participant();
	virtual ~Participant();

	ParticipantImpl* mp_impl;

public:

	/**
	*	Get the GUID_t of the associated RTPSParticipant.
	* @return GUID_t
	*/
	const GUID_t& getGuid()const ;

	/**
	* Get the ParticipantAttributes.
	* @return ParticipantAttributes.
	*/
	const ParticipantAttributes& getAttributes();
	/**
	* Called when using a StaticEndpointDiscovery mechanism different that the one
	* included in FastRTPS, for example when communicating with other implementations.
	* It indicates to the Participant that an Endpoint from the XML has been discovered and
	* should be activated.
	* @param partguid Participant GUID_t.
	* @param userId User defined ID as shown in the XML file.
	* @param kind EndpointKind (WRITER or READER)
	* @return True if correctly found and activated.
	*/
	bool newRemoteEndpointDiscovered(const GUID_t& partguid, uint16_t userId,
		EndpointKind_t kind);

   void add_flow_filter(std::unique_ptr<FlowFilter> filter);
};

}
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
