/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLP.h
 *
 */

#ifndef WLP_H_
#define WLP_H_

#include <vector>

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/common/types/Locator.h"

#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/common/types/Locator.h"

#include "eprosimartps/builtin/liveliness/WLPListener.h"
#include "eprosimartps/builtin/liveliness/WLPTopicDataType.h"

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class StatefulWriter;
class StatefulReader;
class RTPSWriter;
class BuiltinProtocols;
class ParticipantProxyData;
class WLivelinessPeriodicAssertion;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESSMODULE
 */
class WLP : public boost::basic_lockable_adapter<boost::recursive_mutex>
{
	friend class WLPListener;
	friend class WLivelinessPeriodicAssertion;
public:
	WLP(ParticipantImpl* p);
	virtual ~WLP();
	/**
	 * Initialize the WLP protocol.
	 * @param prot Pointer to the BuiltinProtocols object.
	 * @return true if the initialziacion was succesful.
	 */
	bool initWL(BuiltinProtocols* prot);
	/**
	 * Create the endpoitns used in the WLP.
	 * @return true if correct.
	 */
	bool createEndpoints();
	/**
	 * Assign the remote endpoints for a newly discovered participant.
	 * @param pdata Pointer to the ParticipantProxyData object.
	 * @return True if correct.
	 */
	bool assignRemoteEndpoints(ParticipantProxyData* pdata);

	void removeRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Add a local writer to the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool addLocalWriter(RTPSWriter* W);
	/**
	 * Remove a local writer from the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool removeLocalWriter(RTPSWriter* W);

	int64_t m_minAutomatic_MilliSec;

	int64_t m_minManParticipant_MilliSec;

	BuiltinProtocols* getBuiltinProtocols(){return mp_builtinProtocols;};

	bool updateLocalWriter(RTPSWriter* W);


private:
	//!Pointer to the local participant.
	ParticipantImpl* mp_participant;
	//!Pointer to the builtinprotocol class.
	BuiltinProtocols* mp_builtinProtocols;
	//!Pointer to the builtinParticipantMEssageWriter.
	StatefulWriter* mp_builtinParticipantMessageWriter;
	//!Pointer to the builtinParticipantMEssageReader.
	StatefulReader* mp_builtinParticipantMessageReader;
	//!Listener object.
	WLPListener m_listener;
	//!Pointer to the periodic assertion timer object for the automatic liveliness writers.
	WLivelinessPeriodicAssertion* mp_livelinessAutomatic;
	//!Pointer to the periodic assertion timer object for the manual by participant liveliness writers.
	WLivelinessPeriodicAssertion* mp_livelinessManParticipant;
	//!List of the writers using automatic liveliness.
	std::vector<RTPSWriter*> m_livAutomaticWriters;
	//!List of the writers using manual by participant liveliness.
	std::vector<RTPSWriter*> m_livManParticipantWriters;
	//!TopicDataType to extract the key.
	WLPTopicDataType m_wlpTopicDataType;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLP_H_ */
