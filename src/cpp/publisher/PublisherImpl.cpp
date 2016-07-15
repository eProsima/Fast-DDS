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

/*
 * Publisher.cpp
 *
 */

#include "PublisherImpl.h"
#include "../participant/ParticipantImpl.h"
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/publisher/PublisherListener.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>

#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <fastrtps/log/Log.h>

using namespace eprosima::fastrtps;
using namespace ::rtps;



static ::rtps::WriteParams WRITE_PARAM_DEFAULT;

PublisherImpl::PublisherImpl(ParticipantImpl* p,TopicDataType*pdatatype,
		PublisherAttributes& att,PublisherListener* listen ):
										mp_participant(p),
										mp_writer(nullptr),
										mp_type(pdatatype),
										m_att(att),
#pragma warning (disable : 4355 )
										m_history(this, pdatatype->m_typeSize, att.topic.historyQos, att.topic.resourceLimitsQos),
										mp_listener(listen),
#pragma warning (disable : 4355 )
										m_writerListener(this),
										mp_userPublisher(nullptr),
										mp_rtpsParticipant(nullptr)
{

	//Check validity of max_message_size
	std::vector<uinte32_t> buffer_sizes;
	if(p->getAttributes().useBuiltinTransports)
		buffer_sizes.push_back(65000); //TODO(Santi): Change hard-coded value for a ref to the real default socket value)
	for(auto it = p->getAttributes().userTransports.begin(); it != p->getAttributes.userTransport.end(); ++it)
	{
		//Cast through available transports and find the minimum	
		if (auto concrete = dynamic_cast<UDPv4TransportDescriptor*> (it))
			buffer_sizes.push_back(concrete->sendBufferSize);
   		if (auto concrete = dynamic_cast<UDPv6TransportDescriptor*> (it))
			buffer_sizes.push_back(concrete->sendBufferSize);
   		if (auto concrete = dynamic_cast<test_UDPv4TransportDescriptor*> (it))
	   		buffer_sizes.push_back(concrete->sendBufferSize);
	}
	//if ok
	
	uint32_t min_size = std::min_element(buffer_sizes.begin(), buffer_sizes.end());
	// If att.maxmessagesize is over min_size or 0, default to min_size. Else, set it as the max.
	maxmessagesize = ( (min_size < att.maxmessagesize) | (at.maxmessagesize == 0) ) ? min_size : att.maxmessagesize;
		
}

PublisherImpl::~PublisherImpl()
{
	logInfo(PUBLISHER,this->getGuid().entityId << " in topic: "<<this->m_att.topic.topicName);
	RTPSDomain::removeRTPSWriter(mp_writer);
	delete(this->mp_userPublisher);
}



bool PublisherImpl::create_new_change(ChangeKind_t changeKind, void* data)
{
    return create_new_change_with_params(changeKind, data, WRITE_PARAM_DEFAULT);
}

bool PublisherImpl::create_new_change_with_params(ChangeKind_t changeKind, void* data, WriteParams &wparams)
{

    /// Preconditions
	if (data == nullptr)
	{
		logError(PUBLISHER, "Data pointer not valid");
		return false;
	}

	if(changeKind == NOT_ALIVE_UNREGISTERED || changeKind == NOT_ALIVE_DISPOSED ||
			changeKind == NOT_ALIVE_DISPOSED_UNREGISTERED)
	{
		if(m_att.topic.topicKind == NO_KEY)
		{
			logError(PUBLISHER,"Topic is NO_KEY, operation not permitted");
			return false;
		}
	}

	InstanceHandle_t handle;
	if(m_att.topic.topicKind == WITH_KEY)
	{
		mp_type->getKey(data,&handle);
	}

	CacheChange_t* ch = mp_writer->new_change(changeKind,handle);
	if(ch != nullptr)
	{
		if(changeKind == ALIVE)
		{
			if(!mp_type->serialize(data,&ch->serializedPayload))
			{
				logWarning(RTPS_WRITER,"RTPSWriter:Serialization returns false";);
				m_history.release_Cache(ch);
				return false;
			}
			else if(ch->serializedPayload.length > mp_type->m_typeSize)
			{
				logWarning(RTPS_WRITER,"Serialized Payload length larger than maximum type size ("<<ch->serializedPayload.length<<"/"<< mp_type->m_typeSize<<")";);
				m_history.release_Cache(ch);
				return false;
			}
			else if(ch->serializedPayload.length == 0)
			{
				logWarning(RTPS_WRITER,"Serialized Payload length must be set to >0 ";);
				m_history.release_Cache(ch);
				return false;
			}
		}

        // If it is big data, frament it.
        uint32_t high_mark = (mp_participant->getAttributes().rtps.sendSocketBufferSize - RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE) > PAYLOAD_MAX_SIZE ? PAYLOAD_MAX_SIZE : (mp_participant->getAttributes().rtps.sendSocketBufferSize - RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE);

        if(ch->serializedPayload.length > high_mark)
        {
            /// Fragment the data.
            // Set the fragment size to the cachechange.
            // Note: high_mark will always be a value that can be casted to uint16_t)
            ch->setFragmentSize((uint16_t)high_mark);
        }

        if(&wparams != &WRITE_PARAM_DEFAULT)
        {
            ch->write_params = wparams;
        }

		if(!this->m_history.add_pub_change(ch))
		{
			m_history.release_Cache(ch);
			return false;
		}

        // Updated sample identity
        if(&wparams != &WRITE_PARAM_DEFAULT)
        {
            wparams.sample_identity().writer_guid(ch->writerGUID);
            wparams.sample_identity().sequence_number(ch->sequenceNumber);
        }

		return true;
	}
	return false;
}


bool PublisherImpl::removeMinSeqChange()
{
	return m_history.removeMinChange();
}

bool PublisherImpl::removeAllChange(size_t* removed)
{
	return m_history.removeAllChange(removed);
}

const GUID_t& PublisherImpl::getGuid()
{
	return mp_writer->getGuid();
}
//
bool PublisherImpl::updateAttributes(PublisherAttributes& att)
{
	bool updated = true;
	bool missing = false;
	if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
	{
		if(att.unicastLocatorList.size() != this->m_att.unicastLocatorList.size() ||
				att.multicastLocatorList.size() != this->m_att.multicastLocatorList.size())
		{
			logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
			updated &= false;
		}
		else
		{
			for(LocatorListIterator lit1 = this->m_att.unicastLocatorList.begin();
					lit1!=this->m_att.unicastLocatorList.end();++lit1)
			{
				missing = true;
				for(LocatorListIterator lit2 = att.unicastLocatorList.begin();
						lit2!= att.unicastLocatorList.end();++lit2)
				{
					if(*lit1 == *lit2)
					{
						missing = false;
						break;
					}
				}
				if(missing)
				{
					logWarning(PUBLISHER,"Locator: "<< *lit1 << " not present in new list");
					logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
				}
			}
			for(LocatorListIterator lit1 = this->m_att.multicastLocatorList.begin();
					lit1!=this->m_att.multicastLocatorList.end();++lit1)
			{
				missing = true;
				for(LocatorListIterator lit2 = att.multicastLocatorList.begin();
						lit2!= att.multicastLocatorList.end();++lit2)
				{
					if(*lit1 == *lit2)
					{
						missing = false;
						break;
					}
				}
				if(missing)
				{
					logWarning(PUBLISHER,"Locator: "<< *lit1<< " not present in new list");
					logWarning(PUBLISHER,"Locator Lists cannot be changed or updated in this version");
				}
			}
		}
	}

	//TOPIC ATTRIBUTES
	if(this->m_att.topic != att.topic)
	{
		logWarning(PUBLISHER,"Topic Attributes cannot be updated");
		updated &= false;
	}
	//QOS:
	//CHECK IF THE QOS CAN BE SET
	if(!this->m_att.qos.canQosBeUpdated(att.qos))
	{
		updated &=false;
	}
	if(updated)
	{
		if(this->m_att.qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
		{
			//UPDATE TIMES:
			StatefulWriter* sfw = (StatefulWriter*)mp_writer;
			sfw->updateTimes(att.times);
		}
		this->m_att.qos.setQos(att.qos,false);
		this->m_att = att;
		//Notify the participant that a Writer has changed its QOS
		mp_rtpsParticipant->updateWriter(this->mp_writer,m_att.qos);
	}


	return updated;
}

void PublisherImpl::PublisherWriterListener::onWriterMatched(RTPSWriter* /*writer*/,MatchingInfo& info)
{
	if(mp_publisherImpl->mp_listener!=nullptr)
		mp_publisherImpl->mp_listener->onPublicationMatched(mp_publisherImpl->mp_userPublisher,info);
}

bool PublisherImpl::clean_history(unsigned int max)
{
    return mp_writer->clean_history(max);
}

bool PublisherImpl::wait_for_all_acked(const Time_t& max_wait)
{
    return mp_writer->wait_for_all_acked(max_wait);
}

