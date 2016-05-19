/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Endpoint.h
 */



#ifndef ENDPOINT_H_
#define ENDPOINT_H_
#include "common/Types.h"
#include "common/Locator.h"
#include "common/Guid.h"

#include "attributes/EndpointAttributes.h"

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
 * Class Endpoint, all entities of the RTPS network derive from this class.
 * Although the RTPSParticipant is also defined as an endpoint in the RTPS specification, in this implementation
 * the RTPSParticipant class **does not** inherit from the endpoint class. Each Endpoint object owns a pointer to the 
 * RTPSParticipant it belongs to.
 * @ingroup COMMON_MODULE
 */
class Endpoint
{
	friend class RTPSParticipantImpl;
protected:
	Endpoint(RTPSParticipantImpl* pimpl,GUID_t& guid,EndpointAttributes& att);
	virtual ~Endpoint();
public:

	/**
	* Get associated GUID
	* @return Associated GUID
	*/
	RTPS_DllAPI inline const GUID_t& getGuid() const { return m_guid; };

	/**
	* Get mutex
	* @return Associated Mutex
	*/
	RTPS_DllAPI inline boost::recursive_mutex* getMutex() const { return mp_mutex; }

	/**
	* Get associated attributes
	* @return Endpoint attributes
	*/
	RTPS_DllAPI inline EndpointAttributes* getAttributes() { return &m_att; }

protected:
	//!Pointer to the RTPSParticipant containing this endpoint.
	RTPSParticipantImpl* mp_RTPSParticipant;
	//!Endpoint GUID
	const GUID_t m_guid;
	//!Endpoint Attributes
	EndpointAttributes m_att;
	//!Endpoint Mutex
	boost::recursive_mutex* mp_mutex;

private:

    Endpoint& operator=(const Endpoint&)NON_COPYABLE_CXX11;

};


}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINT_H_ */
