/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Endpoint.h
 */



#ifndef ENDPOINT_H_
#define ENDPOINT_H_
#include "eprosimartps/rtps/common/Types.h"
#include "eprosimartps/rtps/common/Locator.h"
#include "eprosimartps/rtps/common/Guid.h"

#include "eprosimartps/rtps/attributes/EndpointAttributes.h"

namespace boost
{
	class recursive_mutex;
}

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class ResourceSend;
class ResourceEvent;


/**
 * Class Endpoint, all entities of the RTPS network are a specification of this class.
 * Although the Participant is also defined as an endpoint in the RTPS specification in this implementation
 * the Participant class DOESN'T inherit from this class. The elements needed where added directly to the
 * Participant class. This way each instance of our class (Endpoint) has a pointer to the participant they belong to.
 * @ingroup COMMONMODULE
 */
class Endpoint
{
public:
	Endpoint(ParticipantImpl* pimpl,GUID_t guid,EndpointAttributes att);
	virtual ~Endpoint();

	inline const GUID_t& getGuid() const {	return m_guid;	};

	inline boost::recursive_mutex* getMutex() const {return mp_mutex;}



	inline EndpointAttributes* getAttributes() {return &m_att;}

protected:
	//!Pointer to the participant containing this endpoints
	ParticipantImpl* mp_participant;
	//! Guid of the Endpoint
	const GUID_t m_guid;
	//!Endpoint Attributes
	EndpointAttributes m_att;
	//!Mutex of the object
	boost::recursive_mutex* mp_mutex;


};



} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
