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
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
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


class EndpointDiscoveryProtocol  {
public:
	EndpointDiscoveryProtocol(ParticipantDiscoveryProtocol* p);
	virtual ~EndpointDiscoveryProtocol();


	virtual bool initEDP(DiscoveryAttributes& attributes)=0;

	virtual bool localWriterMatching(RTPSWriter* writer,bool first_time)=0;
	virtual bool localReaderMatching(RTPSReader* reader,bool first_time)=0;

	virtual void assignRemoteEndpoints(DiscoveredParticipantData* pdata)=0;

protected:
	ParticipantDiscoveryProtocol* mp_PDP;
	//Participant* mp_participant;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* ENDPOINTDISCOVERYPROTOCOL_H_ */
