/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.cpp
 *
 */

#include "eprosimartps/rtps/participant/RTPSParticipant.h"
#include "eprosimartps/rtps/participant/RTPSParticipantImpl.h"
#include "eprosimartps/rtps/Endpoint.h"

namespace eprosima {
namespace rtps {

RTPSParticipant::RTPSParticipant(RTPSParticipantImpl* pimpl):mp_impl(pimpl)
{
};
RTPSParticipant::~RTPSParticipant()
{

};

const GUID_t& RTPSParticipant::getGuid() const
{
	return mp_impl->getGuid();
};

void RTPSParticipant::announceRTPSParticipantState()
{
	return mp_impl->announceRTPSParticipantState();
};

//void RTPSParticipant::loose_next_change()
//{
//	return mp_impl->loose_next_change();
//};

void RTPSParticipant::stopRTPSParticipantAnnouncement()
{
	return mp_impl->stopRTPSParticipantAnnouncement();
};

void RTPSParticipant::resetRTPSParticipantAnnouncement()
{
	return mp_impl->resetRTPSParticipantAnnouncement();
};

bool RTPSParticipant::newRemoteWriterDiscovered(const GUID_t& pguid, int16_t userDefinedId)
{
	return mp_impl->newRemoteEndpointDiscovered(pguid,userDefinedId, WRITER);
}
bool RTPSParticipant::newRemoteReaderDiscovered(const GUID_t& pguid, int16_t userDefinedId)
{
	return mp_impl->newRemoteEndpointDiscovered(pguid,userDefinedId, READER);
}
uint32_t RTPSParticipant::getRTPSParticipantID() const
{
	return mp_impl->getRTPSParticipantID();
}



} /* namespace rtps */
} /* namespace eprosima */


