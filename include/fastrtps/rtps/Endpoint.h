/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Endpoint.h
 */



#ifndef ENDPOINT_H_
#define ENDPOINT_H_
#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/common/Locator.h"
#include "fastrtps/rtps/common/Guid.h"

#include "fastrtps/rtps/attributes/EndpointAttributes.h"

namespace boost
{
	class recursive_mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class RTPSParticipantImpl;
class ResourceSend;
class ResourceEvent;


/**
 * Class Endpoint, all entities of the RTPS network are a specification of this class.
 * Although the RTPSParticipant is also defined as an endpoint in the RTPS specification in this implementation
 * the RTPSParticipant class DOESN'T inherit from this class. The elements needed where added directly to the
 * RTPSParticipant class. This way each instance of our class (Endpoint) has a pointer to the RTPSParticipant they belong to.
 * @ingroup COMMONMODULE
 */
class Endpoint
{
public:
	Endpoint(RTPSParticipantImpl* pimpl,GUID_t& guid,EndpointAttributes& att);
	virtual ~Endpoint();

	inline const GUID_t& getGuid() const {	return m_guid;	};

	inline boost::recursive_mutex* getMutex() const {return mp_mutex;}



	inline EndpointAttributes* getAttributes() {return &m_att;}

protected:
	//!Pointer to the RTPSParticipant containing this endpoints
	RTPSParticipantImpl* mp_RTPSParticipant;
	//! Guid of the Endpoint
	const GUID_t m_guid;
	//!Endpoint Attributes
	EndpointAttributes m_att;
	//!Mutex of the object
	boost::recursive_mutex* mp_mutex;


};


}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
