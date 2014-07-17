/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterLiveliness.h
 *
 */

#ifndef WRITERLIVELINESS_H_
#define WRITERLIVELINESS_H_

#include <vector>

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/common/types/Locator.h"

#include "eprosimartps/liveliness/WriterLivelinessListener.h"

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class StatefulWriter;
class StatefulReader;
class RTPSWriter;
class LivelinessPeriodicAssertion;
class WriterProxy;
class DiscoveredParticipantData;

class WriterLiveliness : public boost::basic_lockable_adapter<boost::recursive_mutex> {
	friend class LivelinessPeriodicAssertion;
	friend class WriterLivelinessListener;
public:
	WriterLiveliness(ParticipantImpl* p);
	virtual ~WriterLiveliness();

	StatefulWriter* mp_builtinParticipantMessageWriter;
	StatefulReader* mp_builtinParticipantMessageReader;

	bool createEndpoints(LocatorList_t& unicastList,LocatorList_t& multicastList);

	bool addLocalWriter(RTPSWriter* W);
	bool removeLocalWriter(RTPSWriter* W);
	bool updateLocalWriter(RTPSWriter* W);


	bool assignRemoteEndpoints(DiscoveredParticipantData* pdata);



	double m_minAutomaticLivelinessPeriod_MilliSec;
	double m_minManualByParticipantLivelinessPeriod_MilliSec;
	ParticipantImpl* mp_participant;
private:

	WriterLivelinessListener m_listener;
	LivelinessPeriodicAssertion* mp_AutomaticLivelinessAssertion;
	LivelinessPeriodicAssertion* mp_ManualByParticipantLivelinessAssertion;
	std::vector<RTPSWriter*> m_AutomaticLivelinessWriters;
	std::vector<RTPSWriter*> m_ManualByParticipantLivelinessWriters;

//	std::vector<WriterProxy*> m_remoteAutomaticLivelinessWriters;
//	std::vector<WriterProxy*> m_remoteManualByParticipantLivelinessWriters;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERLIVELINESS_H_ */
