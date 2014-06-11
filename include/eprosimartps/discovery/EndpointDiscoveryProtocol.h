/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EndpointDiscoveryProtocol.h
 *
 */

#ifndef ENDPOINTDISCOVERYPROTOCOL_H_
#define ENDPOINTDISCOVERYPROTOCOL_H_


#include "eprosimartps/dds/attributes/TopicAttributes.h"
#include "eprosimartps/dds/attributes/ParticipantAttributes.h"


namespace eprosima {
namespace rtps {

class Endpoint;
class RTPSReader;
class RTPSWriter;
class Participant;
class ParticipantDiscoveryProtocol;
class DiscoveredParticipantData;

/**
 * Class EndpointDiscoveryProtocol, base class for the two different EDP classes created for this RTPS implementation.
 * @ingroup DISCOVERYMODULE
 */
class EndpointDiscoveryProtocol  {
public:
	EndpointDiscoveryProtocol(ParticipantDiscoveryProtocol* p);
	virtual ~EndpointDiscoveryProtocol();

	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	virtual bool initEDP(DiscoveryAttributes& attributes)=0;

	/**
	 * Local Writer Matching methods.
	 * @param writer Pointer to the corresponding Writer.
	 * @param first_time Boolean variable indicating whether is the first time this method is called (upon creation of the endpoint).
	 * @return True if correct.
	 *
	 */
	virtual bool localWriterMatching(RTPSWriter* writer,bool first_time)=0;
	/**
	 * Local Reader Matching methods.
	 * @param reader Pointer to the corresponding Reader.
	 * @param first_time Boolean variable indicating whether is the first time this method is called (upon creation of the endpoint).
	 * @return True if correct.
	 *
	 */
	virtual bool localReaderMatching(RTPSReader* reader,bool first_time)=0;
	/**
	 * Method that assigns the remote endpoints to the EDP Endpoints created in this Participant after a new Participant is discovered.
	 * @param pdata Pointer to the DPD object.
	 */
	virtual void assignRemoteEndpoints(DiscoveredParticipantData* pdata)=0;

protected:
	ParticipantDiscoveryProtocol* mp_PDP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINTDISCOVERYPROTOCOL_H_ */
