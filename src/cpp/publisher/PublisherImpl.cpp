/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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

#include <fastrtps/utils/RTPSLog.h>

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {


static const char* const CLASS_NAME = "PublisherImpl";

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

}

PublisherImpl::~PublisherImpl()
{
	const char* const METHOD_NAME = "~PublisherImpl";
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
	const char* const METHOD_NAME = "create_new_change";

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
        uint32_t high_mark = (mp_participant->getAttributes().rtps.sendSocketBufferSize - 106) > PAYLOAD_MAX_SIZE ? PAYLOAD_MAX_SIZE : (mp_participant->getAttributes().rtps.sendSocketBufferSize - 106);

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
	const char* const METHOD_NAME = "updateAttributes";
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

} /* namespace pubsub */
} /* namespace eprosima */


