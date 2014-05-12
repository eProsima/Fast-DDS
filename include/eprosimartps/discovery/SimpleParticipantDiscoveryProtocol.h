/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleParticipantDiscoveryProtocol.h
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
#include "eprosimartps/qos/QosList.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"

#include "eprosimartps/discovery/SPDPListener.h"

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;



/**
 * Class SimpleParticipantDiscoveryProtocol, it announces the participant to predefined Locators
 *  in the network as well as to all other Participants that have been matched.
 *  @ingroup DISCOVERYMODULE
 */
class SimpleParticipantDiscoveryProtocol: public boost::basic_lockable_adapter<boost::recursive_mutex> {
	friend class ResendDiscoveryDataPeriod;
	friend class StaticEndpointDiscoveryProtocol;
public:
	SimpleParticipantDiscoveryProtocol(Participant* p);
	virtual ~SimpleParticipantDiscoveryProtocol();

	/**
	 * Initialization method. Is called by the participant if the ParaticipantAttributes related parameter is set to YES, indicating that the SPDP will be used.
	 * @param[in] domainId DomainId that will be used for this participant.
	 * @param[in] participantId ParticipantId within the DomainParticipant.
	 * @param[in] resendDataPeriod_sec The period for the ResendDiscoveryDataPeriod TimedEvent.
	 * @return True if correct.
	 */
	bool initSPDP(uint16_t domainId,uint16_t participantId,uint16_t resendDataPeriod_sec);

	/**
	 * Method thata sends a DiscoveryParticipantData Message to all added locators.
	 * @return
	 */
	bool sendDPDMsg();


	/**
	 * Check if the DPD data has been changed.
	 * @return
	 */
	bool HasChangedDpd() const {
		return m_hasChanged_DPD;
	}
	/**
	 * Set the variable that indicates that the DPD data has changed.
	 * @param[in] hasChangedDpd
	 */
	void setHasChangedDpd(bool hasChangedDpd) {
		m_hasChanged_DPD = hasChangedDpd;
	}

	bool updateLocalParticipantEntityInfo();

	/**
	 *  This method is called when a new CacheChange is added to the SPDP Reader in order to analyze it.
	 */
	void new_change_added();

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
	bool m_first;
	/**
	 * This method updated the DPD Msg in case something has change in our own participant.
	 * @return True if correct.
	 */
	bool updateDPDMsg();
	bool updateParamList();



	bool processParameterList(ParameterList_t& param, DiscoveredParticipantData* Pdata);

	std::vector<std::string> getMatchedParticipantsNames();

public:
	bool m_useStaticEDP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDISCOVERYPARTICIPANTPROTOCOL_H_ */
