/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.cpp
 *
 */

#include "eprosimartps/rtps/Participant.h"
#include "eprosimartps/rtps/ParticipantImpl.h"
#include "eprosimartps/rtps/Endpoint.h"

namespace eprosima {
namespace rtps {

Participant::Participant(ParticipantImpl* pimpl):mp_impl(pimpl)
{
};
Participant::~Participant()
{

};

const GUID_t& Participant::getGuid() const
{
	return mp_impl->getGuid();
};

void Participant::announceParticipantState()
{
	return mp_impl->announceParticipantState();
};

void Participant::loose_next_change()
{
	return mp_impl->loose_next_change();
};

void Participant::stopParticipantAnnouncement()
{
	return mp_impl->stopParticipantAnnouncement();
};

void Participant::resetParticipantAnnouncement()
{
	return mp_impl->resetParticipantAnnouncement();
};

bool Participant::newRemoteWriterDiscovered(const GUID_t& pguid, int16_t userDefinedId)
{
	return mp_impl->newRemoteEndpointDiscovered(pguid,userDefinedId, WRITER);
}
bool Participant::newRemoteReaderDiscovered(const GUID_t& pguid, int16_t userDefinedId)
{
	return mp_impl->newRemoteEndpointDiscovered(pguid,userDefinedId, READER);
}
uint32_t Participant::getParticipantID() const
{
	return mp_impl->getParticipantID();
}



} /* namespace rtps */
} /* namespace eprosima */


