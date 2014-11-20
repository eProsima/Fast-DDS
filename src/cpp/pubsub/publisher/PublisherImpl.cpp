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

#include "eprosimartps/pubsub/publisher/Publisher.h"
#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/pubsub/publisher/PublisherListener.h"
#include "eprosimartps/rtps/writer/RTPSWriter.h"


#include "eprosimartps/utils/RTPSLog.h"


//#include "eprosimartps/RTPSParticipant.h"

namespace eprosima {
namespace pubsub {


static const char* const CLASS_NAME = "PublisherImpl";

PublisherImpl::PublisherImpl(RTPSParticipantImpl* p,RTPSWriter* Win,TopicDataType*pdatatype,PublisherAttributes& att):
										mp_Writer(Win),
										mp_type(pdatatype),
										m_attributes(att),
										mp_RTPSParticipant(p)
{

}

PublisherImpl::~PublisherImpl() {
	const char* const METHOD_NAME = "~PublisherImpl";
	logInfo(RTPS_WRITER,"OK");
}

bool PublisherImpl::write(void* Data) {
	const char* const METHOD_NAME = "write";
	logInfo(RTPS_WRITER,"Writing new data");
	return mp_Writer->add_new_change(ALIVE,Data);
}

bool PublisherImpl::dispose(void* Data)
{
	const char* const METHOD_NAME = "dispose";
	logInfo(RTPS_WRITER,"Disposing of Data");
	return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED,Data);
}


bool PublisherImpl::unregister(void* Data) {
	const char* const METHOD_NAME = "unregister";
	//Convert data to serialized Payload
	logInfo(RTPS_WRITER,"Unregistering of Data");
	return mp_Writer->add_new_change(NOT_ALIVE_UNREGISTERED,Data);
}

bool PublisherImpl::dispose_and_unregister(void* Data) {
	//Convert data to serialized Payload
	const char* const METHOD_NAME = "dispose_and_unregister";
	logInfo(RTPS_WRITER,"Disposing and Unregistering Data");
	return mp_Writer->add_new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,Data);
}


bool PublisherImpl::add_new_change(ChangeKind_t changeKind, void* data)
{
	if(mp_type->m_isGetKeyDefined)
					{
						mp_type->getKey(data,&ch->instanceHandle);
					}
					else
					{
						logWarning(RTPS_WRITER,"Get key function not defined";);
					}

	CacheChange_t * ch = mp_writer->new_change(changeKind);
	if(ch != nullptr)
	{
		if(changeKind == ALIVE && data !=nullptr && mp_type !=nullptr)
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
				m_writer_cache.release_Cache(ch);
				return false;
			}
			else if(ch->serializedPayload.length == 0)
			{
				logWarning(RTPS_WRITER,"Serialized Payload length must be set to >0 ";);
				m_writer_cache.release_Cache(ch);
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


