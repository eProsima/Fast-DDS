/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.cpp
 *
 */

#include "eprosimartps/pubsub/publisher/PublisherImpl.h"
#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/pubsub/publisher/PublisherListener.h"
#include "eprosimartps/rtps/writer/RTPSWriter.h"


#include "eprosimartps/utils/RTPSLog.h"


//#include "eprosimartps/RTPSParticipant.h"

namespace eprosima {
namespace pubsub {


static const char* const CLASS_NAME = "PublisherImpl";

PublisherImpl::PublisherImpl(PUBSUBParticipant* p,RTPSWriter* Win,TopicDataType*pdatatype,PublisherAttributes& att):
										mp_writer(Win),
										mp_type(pdatatype),
										m_att(att),
										mp_RTPSParticipant(p)
{

}

PublisherImpl::~PublisherImpl() {
	const char* const METHOD_NAME = "~PublisherImpl";
	logInfo(RTPS_WRITER,"OK");
}




bool PublisherImpl::create_new_change(ChangeKind_t changeKind, void* data)
{
	const char* const METHOD_NAME = "create_new_change";
	if (data == nullptr)
	{
		logError(PUBSUB_PUBLISHER, "Data pointer not valid");
		return false;
	}
	if(changeKind == NOT_ALIVE_UNREGISTERED || changeKind == NOT_ALIVE_DISPOSED ||
			changeKind == NOT_ALIVE_DISPOSED_UNREGISTERED)
	{
		if(m_att.topic.topicKind == NO_KEY)
		{
			logError(PUBSUB_PUBLISHER,"Topic is NO_KEY, operation not permitted");
			return false;
		}
	}
	InstanceHandle_t handle;
	if(m_att.topic.topicKind == WITH_KEY)
	{
		mp_type->getKey(data,&handle);
	}
	CacheChange_t * ch = mp_writer->new_change(changeKind,handle);
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
	}
}


bool PublisherImpl::removeMinSeqChange()
{
	return mp_Writer->removeMinSeqCacheChange();
}

bool PublisherImpl::removeAllChange(size_t* removed)
{
	return mp_Writer->removeAllCacheChange(removed);
}

size_t PublisherImpl::getHistoryElementsNumber()
{
	return mp_Writer->getHistoryCacheSize();
}



const GUID_t& PublisherImpl::getGuid()
{
	return mp_Writer->getGuid();
}

bool PublisherImpl::updateAttributes(PublisherAttributes& att)
{
	const char* const METHOD_NAME = "updateAttributes";
	bool updated = true;
	bool missing = false;
	if(this->mp_Writer->getStateType() == STATEFUL)
	{
		if(att.unicastLocatorList.size() != this->m_attributes.unicastLocatorList.size() ||
				att.multicastLocatorList.size() != this->m_attributes.multicastLocatorList.size())
		{
			logWarning(RTPS_WRITER,"Locator Lists cannot be changed or updated in this version");
			updated &= false;
		}
		else
		{
			for(LocatorListIterator lit1 = this->m_attributes.unicastLocatorList.begin();
					lit1!=this->m_attributes.unicastLocatorList.end();++lit1)
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
					logWarning(RTPS_WRITER,"Locator: "<< *lit1 << " not present in new list");
					logWarning(RTPS_WRITER,"Locator Lists cannot be changed or updated in this version");
				}
			}
			for(LocatorListIterator lit1 = this->m_attributes.multicastLocatorList.begin();
					lit1!=this->m_attributes.multicastLocatorList.end();++lit1)
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
					logWarning(RTPS_WRITER,"Locator: "<< *lit1<< " not present in new list");
					logWarning(RTPS_WRITER,"Locator Lists cannot be changed or updated in this version");
				}
			}
		}
	}

	//TOPIC ATTRIBUTES
	if(this->m_attributes.topic != att.topic)
	{
		logWarning(RTPS_WRITER,"Topic Attributes cannot be updated");
		updated &= false;
	}
	//QOS:
	//CHECK IF THE QOS CAN BE SET
	if(!this->mp_Writer->canQosBeUpdated(att.qos))
	{
		updated &=false;
	}
	if(updated)
	{
		if(this->mp_Writer->getStateType() == STATEFUL)
		{
			//UPDATE TIMES:
			StatefulWriter* sfw = (StatefulWriter*)mp_Writer;
			sfw->updateTimes(att.times);
		}
		this->mp_Writer->setQos(att.qos,false);
		this->m_attributes = att;
		//NOTIFY THE BUILTIN PROTOCOLS THAT THE READER HAS CHANGED
		mp_RTPSParticipant->getBuiltinProtocols()->updateLocalWriter(this->mp_Writer);
	}


	return updated;
}


} /* namespace pubsub */
} /* namespace eprosima */


