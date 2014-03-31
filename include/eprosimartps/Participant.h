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
#include "eprosimartps/threadtype/ThreadEvent.h"
#include "eprosimartps/threadtype/ThreadListen.h"
#include "eprosimartps/threadtype/ThreadSend.h"



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



/**
 * @class Participant
 * @brief Class Participant, it contains all the entities and allows the creation and removal of writers and readers. It manages the send and receive threads.
 * @ingroup MANAGEMENTMODULE
 */
class Participant{
	friend class ThreadSend;
	friend class ThreadListen;
	friend class eprosima::dds::DomainParticipant;
private:

	RTPS_DllAPI Participant(const ParticipantParams_t&param);
	virtual ~Participant();

	/**
	 * Create a StatelessWriter from a parameter structure.
	 * @param[out] SWriter Pointer to the stateless writer.
	 * @param[in] Wparam Parameters to use in the creation.
	 * @return True if correct.
	 */
	bool createStatelessWriter(StatelessWriter** SWriter,const WriterParams_t& Wparam,uint32_t payload_size);

	bool createStatefulWriter(StatefulWriter** SWriter, const WriterParams_t& Wparam,uint32_t payload_size);

	bool initWriter(RTPSWriter* W);

	/**
	 * Create a StatelessReader from a parameter structure and add it to the participant.
	 * @param[out] SReader Pointer to the stateless reader.
	 * @param[in] RParam Parameters to use in the creation.
	 * @return True if correct.
	 */
	bool createStatelessReader(StatelessReader** SReader,const ReaderParams_t& RParam,uint32_t payload_size);
	bool createStatefulReader(StatefulReader** SReader,const ReaderParams_t& RParam,uint32_t payload_size);
	bool initReader(RTPSReader* R);


	/**
	 * Remove Endpoint from the participant. It closes all entities related to them that are no longer in use.
	 * For example, if a ThreadListen is not useful anymore the thread is closed and the instance removed.
	 * @param[in] endpoint Pointer to the Endpoint that is going to be removed.
	 * @return True if correct.
	 */
	bool removeEndpoint(Endpoint* endpoint);

//	//!Protocol Version used by this participant.
//	ProtocolVersion_t protocolVersion;
//	//!VendodId of the participant.
//	VendorId_t vendorId;
	//!Default listening addresses.
	std::vector<Locator_t> m_defaultUnicastLocatorList;
	//!Default listening addresses.
	std::vector<Locator_t> m_defaultMulticastLocatorList;
public:
	//!Guid of the participant.
	GUID_t m_guid;

	//! Sending resources.
	ThreadSend m_send_thr;

	ThreadEvent m_event_thr;

private:
	//!Semaphore to wait for the listen thread creation.
	boost::interprocess::interprocess_semaphore* m_endpointToListenThreadSemaphore;
	//!Id counter to correctly assign the ids to writers and readers.
	uint32_t IdCounter;
	//!Writer List.
	std::vector<RTPSWriter*> m_writerList;
	//!Reader List
	std::vector<RTPSReader*> m_readerList;
	//!Listen thread list.
	std::vector<ThreadListen*> m_threadListenList;
	/*!
	 * Assign a given Endpoint to one of the current listen thread or create a new one.
	 * @param[in] endpoint Pointer to the Endpoint to add.
	 * @param[in] type Type of the Endpoint (R or W)(Reader or Writer).
	 * @return True if correct.
	 */
	bool assignEnpointToListenThreads(Endpoint* endpoint,char type);
	/*!
	 * Create a new listen thread in the specified locator.
	 * @param[in] loc Locator to use.
	 * @param[out] listenthread Pointer to pointer of this class to correctly initialize the listening recourse.
	 * @return True if correct.
	 */
	bool addNewListenThread(Locator_t& loc,ThreadListen** listenthread);
public:
	//!Used for tests
	void loose_next_change(){m_send_thr.m_send_next = false;};

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */





