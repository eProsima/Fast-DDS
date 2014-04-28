/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipantProtocol.h
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_
#define SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_


#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/timedevent/ResendDiscoveryDataPeriod.h"
#include "eprosimartps/dds/QosList.h"
#include "eprosimartps/discovery/DiscoveredParticipantData.h"

#include "eprosimartps/discovery/SPDPListener.h"

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;




class SimpleParticipantDiscoveryProtocol: public boost::basic_lockable_adapter<boost::recursive_mutex> {
	friend class ResendDiscoveryDataPeriod;
	friend class StaticEndpointDiscoveryProtocol;
public:
	SimpleParticipantDiscoveryProtocol(Participant* p);
	virtual ~SimpleParticipantDiscoveryProtocol();

	bool initSPDP(uint16_t domainId,uint16_t participantId,uint16_t resendDataPeriod_sec);


	bool sendDPDMsg();
	bool updateDPDMsg();
	bool updateParamList();

	void new_change_added();

	bool processParameterList(ParameterList_t& param, DiscoveredParticipantData* Pdata);


	std::vector<std::string> getMatchedParticipantsNames();
private:
	Participant* mp_Participant;
	StatelessWriter* m_SPDPbPWriter;
	StatelessReader* m_SPDPbPReader;
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	//Locator_t m_defaultMulticastLocator;
	uint16_t m_domainId;
	ResendDiscoveryDataPeriod* m_resendData;
	CDRMessage_t m_DPDMsgHeader;
	CDRMessage_t m_DPDMsgData;
	CDRMessage_t m_DPDMsg;
	bool m_hasChanged_DPD;
	DiscoveredParticipantData m_DPD;
	QosList_t m_DPDAsParamList;
	std::vector<DiscoveredParticipantData*> m_matched_participants;
	SPDPListener m_listener;
public:
	bool m_useStaticEDP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_ */
