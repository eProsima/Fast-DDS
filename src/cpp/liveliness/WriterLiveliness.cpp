/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterLiveliness.cpp
 *
 */

#include "eprosimartps/common/types/Guid.h"

#include "WriterLiveliness.h"


#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

WriterLiveliness::WriterLiveliness(ParticipantImpl* p):
		mp_participant(p),
		mp_builtinParticipantMessageWriter(NULL),
		mp_builtinParticipantMessageReader(NULL)
{
	// TODO Auto-generated constructor stub

}

WriterLiveliness::~WriterLiveliness() {
	// TODO Auto-generated destructor stub
}

bool WriterLiveliness::createEndpoints()
{
	//CREATE WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	Wparam.historyMaxSize = 1;
	Wparam.payloadMaxSize = 20;
	Wparam.unicastLocatorList = mp_participant->m_defaultUnicastLocatorList;
	Wparam.multicastLocatorList = mp_participant->m_defaultMulticastLocatorList;
	Wparam.topic.topicName = "DCPSParticipantMessage";
	Wparam.topic.topicDataType = "ParticipantMessageData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.userDefinedId = -1;
	Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSWriter* out;
	if(mp_participant->createWriter(&out,Wparam,Wparam.payloadMaxSize,true,STATEFUL,NULL,NULL,c_EntityId_WriterLiveliness))
	{

	}
	else
	{
		pError("Liveliness Writer Creation failed "<<endl;)
				return false;
	}


}

} /* namespace rtps */
} /* namespace eprosima */
