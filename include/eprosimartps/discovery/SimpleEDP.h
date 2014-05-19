/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleEDP.h
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEEDP_H_
#define SIMPLEEDP_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;

class SimpleEDP: public eprosima::rtps::EndpointDiscoveryProtocol {
public:
	SimpleEDP();
	virtual ~SimpleEDP();

	bool initEDP(DiscoveryAttributes& attributes);

	DiscoveryAttributes m_discovery;

	StatefulWriter* mp_PubWriter;
	StatefulWriter* mp_SubWriter;
	StatefulWriter* mp_TopWriter;
	StatefulReader* mp_PubReader;
	StatefulReader* mp_SubReader;
	StatefulReader* mp_TopReader;


	bool localWriterMatching(RTPSWriter* writer,bool first_time);
	bool localReaderMatching(RTPSReader* reader,bool first_time);

	bool createSEDPEndpoints();

	void assignRemoteEndpoints(DiscoveredParticipantData* pdata);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEEDP_H_ */
