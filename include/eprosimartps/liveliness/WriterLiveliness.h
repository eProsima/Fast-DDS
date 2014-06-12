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
#include "eprosimartps/common/types/Time_t.h"

#include "eprosimartps/liveliness/WriterLivelinessListener.h"


namespace eprosima {
namespace rtps {

class ParticipantImpl;
class StatefulWriter;
class StatefulReader;
class RTPSWriter;
class LivelinessPeriodicAssertion;

class WriterLiveliness {
	friend class LivelinessPeriodicAssertion;
public:
	WriterLiveliness(ParticipantImpl* p);
	virtual ~WriterLiveliness();

	StatefulWriter* mp_builtinParticipantMessageWriter;
	StatefulReader* mp_builtinParticipantMessageReader;

	bool createEndpoints();

	bool addWriter(RTPSWriter* W);
	bool removeWriter(RTPSWriter* W);
	bool updateWriter(RTPSWriter* W);

	double m_minAutomaticLivelinessLeaseDuration_MilliSec;
	double m_minManualByParticipantLivelinessLeaseDuration_MilliSec;
	ParticipantImpl* mp_participant;
private:

	WriterLivelinessListener m_listener;
	LivelinessPeriodicAssertion* mp_AutomaticLivelinessAssertion;
	LivelinessPeriodicAssertion* mp_ManualByParticipantLivelinessAssertion;
	std::vector<RTPSWriter*> m_AutomaticLivelinessWriters;
	std::vector<RTPSWriter*> m_ManualByParticipantLivelinessWriters;

	std::vector<WriterProxy*> m_remoteAutomaticLivelinessWriters;
	std::vector<WriterProxy*> m_remoteManualByParticipantLivelinessWriters;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERLIVELINESS_H_ */
