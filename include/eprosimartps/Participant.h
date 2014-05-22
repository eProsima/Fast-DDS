/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.h
 *	Participant class definition.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "eprosimartps/rtps_all.h"

#include "eprosimartps/common/attributes/TopicAttributes.h"
#include "eprosimartps/common/attributes/ReliabilityAttributes.h"
#include "eprosimartps/common/attributes/PublisherAttributes.h"
#include "eprosimartps/common/attributes/SubscriberAttributes.h"
#include "eprosimartps/common/attributes/ParticipantAttributes.h"

#include "eprosimartps/resources/ResourceEvent.h"
#include "eprosimartps/resources/ResourceListen.h"
#include "eprosimartps/resources/ResourceSend.h"
//#include "eprosimartps/discovery/SimpleParticipantDiscoveryProtocol.h"
//#include "eprosimartps/discovery/StaticEndpointDiscoveryProtocol.h"





#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

namespace eprosima {

namespace dds{
class DomainParticipant;
}

namespace rtps {

class StatelessWriter;
class StatelessReader;
class StatefulWriter;
class StatefulReader;
class RTPSReader;
class RTPSWriter;
class Endpoint;
class ParticipantDiscoveryProtocol;

/**
 * @class Participant
 * @brief Class Participant, it contains all the entities and allows the creation and removal of writers and readers. It manages the send and receive threads.
 * @ingroup MANAGEMENTMODULE
 */
class Participant{
	friend class ResourceSend;
	friend class ResourceListen;
	friend class eprosima::dds::DomainParticipant;
	friend class ParticipantDiscoveryProtocol;
	friend class EndpointDiscoveeryProtocol;
	friend class SimplePDP;
	friend class StaticEDP;
	friend class SimpleEDP;
	friend class SPDPListener;
	friend class SEDPListeners;
	friend class SEDPPubListener;
	friend class SEDPSubListener;
private:

	Participant(const ParticipantAttributes &param,uint32_t id);
	virtual ~Participant();

	/**
	 * Create a StatelessWriter from a parameter structure.
	 * @param[out] SWriter Pointer to the stateless writer.
	 * @param[in] Wparam Parameters to use in the creation.
	 * @param[in] payload_size Size of the payload in this writer.
	 * @return True if correct.
	 */
	bool createStatelessWriter(StatelessWriter** SWriter, PublisherAttributes& Wparam,uint32_t payload_size,bool isBuiltin);
	/**
	 * Create a StatefulWriter from a parameter structure.
	 * @param[out] SWriter Pointer to the stateful writer.
	 * @param[in] Wparam Parameters to use in the creation.
	 * @param[in] payload_size Size of the payload in this writer.
	 * @return True if correct.
	 */
	bool createStatefulWriter(StatefulWriter** SWriter,  PublisherAttributes& Wparam,uint32_t payload_size,bool isBuiltin);

	bool initWriter(RTPSWriter* W,bool isBuiltin);

	/**
	 * Create a StatelessReader from a parameter structure and add it to the participant.
	 * @param[out] SReader Pointer to the stateless reader.
	 * @param[in] RParam Parameters to use in the creation.
	 * @param[in] payload_size Size of the payload associated with this Reader.
	 * @return True if correct.
	 */
	bool createStatelessReader(StatelessReader** SReader, SubscriberAttributes& RParam,uint32_t payload_size,bool isBuiltin);
	/**
		 * Create a StatefulReader from a parameter structure and add it to the participant.
		 * @param[out] SReader Pointer to the stateful reader.
		 * @param[in] RParam Parameters to use in the creation.
		 * @param[in] payload_size Size of the payload associated with this Reader.
		 * @return True if correct.
		 */
	bool createStatefulReader(StatefulReader** SReader, SubscriberAttributes& RParam,uint32_t payload_size,bool isBuiltin);

	bool initReader(RTPSReader* R,bool isBuiltin);


	/**
	 * Remove Endpoint from the participant. It closes all entities related to them that are no longer in use.
	 * For example, if a ResourceListen is not useful anymore the thread is closed and the instance removed.
	 * @param[in] endpoint Pointer to the Endpoint that is going to be removed.
	 * @param[in] type Char indicating if it is Reader ('R') or Writer ('W')
	 * @return True if correct.
	 */
	bool removeUserEndpoint(Endpoint* p_endpoint,char type);

//	//!Protocol Version used by this participant.
//	ProtocolVersion_t protocolVersion;
//	//!VendodId of the participant.
//	VendorId_t vendorId;
	//!Default listening addresses.
	LocatorList_t m_defaultUnicastLocatorList;
	//!Default listening addresses.
	LocatorList_t m_defaultMulticastLocatorList;

	//SimpleParticipantDiscoveryProtocol m_SPDP;
	std::string m_participantName;
	//StaticEndpointDiscoveryProtocol m_StaticEDP;
public:
	//!Guid of the participant.
	GUID_t m_guid;

	//! Sending resources.
	ResourceSend m_send_thr;

	ResourceEvent m_event_thr;

	std::vector<RTPSReader*>::iterator userReadersListBegin(){return m_userReaderList.begin();};
	std::vector<RTPSReader*>::iterator userReadersListEnd(){return m_userReaderList.end();};
	std::vector<RTPSWriter*>::iterator userWritersListBegin(){return m_userWriterList.begin();};
	std::vector<RTPSWriter*>::iterator userWritersListEnd(){return m_userWriterList.end();};

private:
	//!Semaphore to wait for the listen thread creation.
	boost::interprocess::interprocess_semaphore* m_ResourceSemaphore;
	//!Id counter to correctly assign the ids to writers and readers.
	uint32_t IdCounter;
	//!Writer List.
	std::vector<RTPSWriter*> m_allWriterList;
	//!Reader List
	std::vector<RTPSReader*> m_allReaderList;
	//!Listen thread list.
	//!Writer List.
	std::vector<RTPSWriter*> m_userWriterList;
	//!Reader List
	std::vector<RTPSReader*> m_userReaderList;
	//!Listen thread list.
	std::vector<ResourceListen*> m_threadListenList;
	/*!
	 * Assign a given Endpoint to one of the current listen thread or create a new one.
	 * @param[in] endpoint Pointer to the Endpoint to add.
	 * @param[in] type Type of the Endpoint (R or W)(Reader or Writer).
	 * @return True if correct.
	 */
	bool assignEnpointToListenResources(Endpoint* endpoint,char type);
	/*!
	 * Create a new listen thread in the specified locator.
	 * @param[in] loc Locator to use.
	 * @param[out] listenthread Pointer to pointer of this class to correctly initialize the listening recourse.
	 * @param[in] isMulticast To indicate whether the new lsited thread is multicast.
	 * @return True if correct.
	 */
	bool addNewListenResource(Locator_t& loc,ResourceListen** listenthread,bool isMulticast);

	ParticipantDiscoveryProtocol* mp_PDP;

public:
	//!Used for tests
	void loose_next_change(){m_send_thr.m_send_next = false;};

	DiscoveryAttributes m_discovery;

	//! Announce ParticipantState
	void announceParticipantState();

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */





