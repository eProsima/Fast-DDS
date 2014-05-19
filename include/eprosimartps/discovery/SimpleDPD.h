/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDPD.h
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEDPD_H_
#define SIMPLEDPD_H_

#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"

namespace eprosima {
namespace rtps {

class SimpleDPD: public eprosima::rtps::ParticipantDiscoveryProtocol {
public:
	SimpleDPD(Participant* p_part);
	virtual ~SimpleDPD();

	bool initDPD(DiscoveryAttributes& attributes,uint32_t participantID);
	bool addLocalParticipant(Participant* p);

	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;

	bool createSPDPEndpoints();

	StatelessWriter* mp_SPDPWriter;
	StatelessReader* mp_SPDPReader;
	SPDPListener m_listener;
	bool m_hasChangedLocalDPD;

	void announceParticipantState(bool new_change);

	bool updateParameterList();

	QosList_t m_localDPDasQosList;

	bool addStaticEDPInfo();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDPD_H_ */
