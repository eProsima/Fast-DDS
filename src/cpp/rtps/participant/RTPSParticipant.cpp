// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file RTPSParticipant.cpp
 *
 */

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include "RTPSParticipantImpl.h"
#include <fastrtps/rtps/Endpoint.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

RTPSParticipant::RTPSParticipant(RTPSParticipantImpl* pimpl):mp_impl(pimpl)
{
}

RTPSParticipant::~RTPSParticipant()
{

}

const GUID_t& RTPSParticipant::getGuid() const
{
	return mp_impl->getGuid();
}

void RTPSParticipant::announceRTPSParticipantState()
{
	return mp_impl->announceRTPSParticipantState();
}

void RTPSParticipant::stopRTPSParticipantAnnouncement()
{
	return mp_impl->stopRTPSParticipantAnnouncement();
}

void RTPSParticipant::resetRTPSParticipantAnnouncement()
{
	return mp_impl->resetRTPSParticipantAnnouncement();
}

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

bool RTPSParticipant::registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos)
{
	return mp_impl->registerWriter(Writer,topicAtt, wqos);
}

bool RTPSParticipant::registerReader(RTPSReader* Reader,TopicAttributes& topicAtt,ReaderQos& rqos)
{
	return mp_impl->registerReader(Reader,topicAtt, rqos);
}

bool RTPSParticipant::updateWriter(RTPSWriter* Writer,WriterQos& wqos)
{
	return mp_impl->updateLocalWriter(Writer, wqos);
}

bool RTPSParticipant::updateReader(RTPSReader* Reader,ReaderQos& rqos)
{
	return mp_impl->updateLocalReader(Reader, rqos);
}

std::pair<StatefulReader*,StatefulReader*> RTPSParticipant::getEDPReaders(){
	
	return mp_impl->getEDPReaders();
}

}
} /* namespace rtps */
} /* namespace eprosima */


