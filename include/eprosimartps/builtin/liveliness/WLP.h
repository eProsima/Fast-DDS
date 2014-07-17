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

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class StatefulWriter;
class StatefulReader;
class RTPSWriter;
class BuiltinProtocols;
class ParticipantProxyData;
class WLivelinessPeriodicAssertion;


class WLP : public boost::basic_lockable_adapter<boost::recursive_mutex>
{
	friend class WLPListener;
	friend class WLivelinessPeriodicAssertion;
public:
	WLP(ParticipantImpl* p);
	virtual ~WLP();

	bool initWL(BuiltinProtocols* prot);

	bool createEndpoints();
	bool assignRemoteEndpoints(ParticipantProxyData* pdata);

	bool addLocalWriter(RTPSWriter* W);
	bool removeLocalWriter(RTPSWriter* W);

	double m_minAutomatic_MilliSec;
	double m_minManParticipant_MilliSec;


private:
	ParticipantImpl* mp_participant;
	BuiltinProtocols* mp_builtinProtocols;
	StatefulWriter* mp_builtinParticipantMessageWriter;
	StatefulReader* mp_builtinParticipantMessageReader;

	WLPListener m_listener;

	WLivelinessPeriodicAssertion* mp_livelinessAutomatic;
	WLivelinessPeriodicAssertion* mp_livelinessManParticipant;
	std::vector<RTPSWriter*> m_livAutomaticWriters;
	std::vector<RTPSWriter*> m_livManParticipantWriters;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLP_H_ */
