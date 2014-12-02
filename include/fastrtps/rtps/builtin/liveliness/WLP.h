/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLP.h
 *
 */

#ifndef WLP_H_
#define WLP_H_

#include <vector>

#include "fastrtps/rtps/common/Time_t.h"
#include "fastrtps/rtps/common/Locator.h"

namespace boost
{
class recursive_mutex;
}


namespace eprosima {
namespace fastrtps{

class WriterQos;


namespace rtps {

class RTPSParticipantImpl;
class StatefulWriter;
class StatefulReader;
class RTPSWriter;
class BuiltinProtocols;
class ParticipantProxyData;
class WLivelinessPeriodicAssertion;
class WLPListener;
class WriterHistory;
class ReaderHistory;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESSMODULE
 */
class WLP
{
	friend class WLPListener;
	friend class WLivelinessPeriodicAssertion;
public:
	/**
	* @param prot
	*/
	WLP(BuiltinProtocols* prot);
	virtual ~WLP();
	/**
	 * Initialize the WLP protocol.
	 * @param prot Pointer to the BuiltinProtocols object.
	 * @return true if the initialziacion was succesful.
	 */
	bool initWL(RTPSParticipantImpl* p);
	/**
	 * Create the endpoitns used in the WLP.
	 * @return true if correct.
	 */
	bool createEndpoints();
	/**
	 * Assign the remote endpoints for a newly discovered RTPSParticipant.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 * @return True if correct.
	 */
	bool assignRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Remove remote endpoints from the liveliness protocol.
	 * @param pdata Pointer to the ParticipantProxyData to remove
	 */
	void removeRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Add a local writer to the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool addLocalWriter(RTPSWriter* W,WriterQos& wqos);
	/**
	 * Remove a local writer from the liveliness protocol.
	 * @param Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool removeLocalWriter(RTPSWriter* W);

	//!
	double m_minAutomatic_MilliSec;
	//!
	double m_minManRTPSParticipant_MilliSec;
	
	/**
	 * Get the builtin protocols
	 * @return Builtin protocols
	 */
	BuiltinProtocols* getBuiltinProtocols(){return mp_builtinProtocols;};
	
	/**
	 * Update local writer.
	 * @param W Writer to update
	 * @param wqos New writer QoS
	 * @return True on success
	 */
	bool updateLocalWriter(RTPSWriter* W,WriterQos& wqos);
	
	/**
	 * Get the RTPS participant
	 * @return RTPS participant
	 */
	inline RTPSParticipantImpl* getRTPSParticipant(){return mp_participant;}
	
	/**
	 * Get the mutex
	 * @return mutex
	 */
	inline boost::recursive_mutex* getMutex() {return mp_mutex;};

private:
	//!Pointer to the local RTPSParticipant.
	RTPSParticipantImpl* mp_participant;
	//!Pointer to the builtinprotocol class.
	BuiltinProtocols* mp_builtinProtocols;
	//!Pointer to the builtinRTPSParticipantMEssageWriter.
	StatefulWriter* mp_builtinWriter;
	//!Pointer to the builtinRTPSParticipantMEssageReader.
	StatefulReader* mp_builtinReader;
	//!Hist
	WriterHistory* mp_builtinWriterHistory;
	ReaderHistory* mp_builtinReaderHistory;
	//!Listener object.
	WLPListener* mp_listener;
	//!Pointer to the periodic assertion timer object for the automatic liveliness writers.
	WLivelinessPeriodicAssertion* mp_livelinessAutomatic;
	//!Pointer to the periodic assertion timer object for the manual by RTPSParticipant liveliness writers.
	WLivelinessPeriodicAssertion* mp_livelinessManRTPSParticipant;
	//!List of the writers using automatic liveliness.
	std::vector<RTPSWriter*> m_livAutomaticWriters;
	//!List of the writers using manual by RTPSParticipant liveliness.
	std::vector<RTPSWriter*> m_livManRTPSParticipantWriters;

	boost::recursive_mutex* mp_mutex;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLP_H_ */
