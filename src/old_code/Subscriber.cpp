/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Subscriber.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/StatefulReader.h"

namespace eprosima {
namespace dds {



Subscriber::Subscriber(RTPSReader* Rin):
		mp_Reader(Rin)
{

}


Subscriber::~Subscriber() {

	pDebugInfo("Subscriber destructor"<<endl;);
}


void Subscriber::waitForUnreadMessage()
{

	mp_Reader->newMessageSemaphore->wait();
}

void Subscriber::assignListener(SubscriberListener* p_listener) {
	mp_Reader->mp_listener = p_listener;
}

bool Subscriber::isHistoryFull()
{
	return mp_Reader->m_reader_cache.isFull();
}

int Subscriber::getReadElements_n()
{
	return readElements.size();
}

int Subscriber::getHistory_n()
{
	return mp_Reader->m_reader_cache.m_changes.size();
}

bool Subscriber::isCacheRead(SequenceNumber_t& seqNum, GUID_t& guid)
{
	std::vector<ReadElement_t>::iterator it;
	for(it=readElements.begin();it!=readElements.end();++it)
	{
		if(seqNum.to64long()== it->seqNum.to64long() &&
				guid == it->writerGuid)
		{
			return true;
		}
	}
	return false;
}
//
//bool Subscriber::readMinSeqUnreadCache(void* data_ptr)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//		if(mp_Reader->m_reader_cache.m_changes.size() == readElements.size())
//		{
//			pWarning( "No unread elements " << endl);
//			return false; //No elements unread
//		}
//		SequenceNumber_t minSeqNum;
//		GUID_t minSeqNumGuid;
//		SEQUENCENUMBER_UNKOWN(minSeqNum);
//
//		if(readElements.empty()) // No element is read yet
//		{
//			mp_Reader->m_reader_cache.get_seq_num_min(&minSeqNum,&minSeqNumGuid);
//		}
//		else
//		{
//			for(std::vector<CacheChange_t*>::iterator it=mp_Reader->m_reader_cache.m_changes.begin();
//					it!=mp_Reader->m_reader_cache.m_changes.end();++it)
//			{
//				if(!isCacheRead((*it)->sequenceNumber,(*it)->writerGUID))
//				{
//					if(minSeqNum.high == -1 ||
//					(*it)->sequenceNumber.to64long() < minSeqNum.to64long())
//					{
//						minSeqNum = (*it)->sequenceNumber;
//						minSeqNumGuid = (*it)->writerGUID;
//					}
//				}
//			}
//		}
//		ReadElement_t rElem;
//		rElem.seqNum = minSeqNum;
//		rElem.writerGuid = minSeqNumGuid;
//		readElements.push_back(rElem);
//		CacheChange_t* ch;
//		mp_Reader->m_reader_cache.get_change(minSeqNum,minSeqNumGuid,&ch);
//		if(ch->kind == ALIVE)
//		{
//			if(!mp_type->deserialize(&ch->serializedPayload,data_ptr))
//			{
//				pWarning("Subscriber:Deserialization returns false"<<endl);
//				return false;
//			}
//		}
//		else
//		{
//			pWarning("Reading a NOT ALIVE change "<<endl);
//		}
//
//		return true;
//	}
//	pWarning("Not read anything. Reader Cache empty" << endl);
//	return false;
//}
//bool Subscriber::readCache(SequenceNumber_t& sn, GUID_t& wGuid,void* data_ptr)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	CacheChange_t* ch = NULL;
//	if(mp_Reader->m_reader_cache.get_change(sn,wGuid,&ch))
//	{
//		if(ch->kind == ALIVE)
//		{
//			if(!mp_type->deserialize(&ch->serializedPayload,data_ptr))
//			{
//				pWarning("Subscriber:Deserialization returns false"<<endl);
//				return false;
//			}
//		}
//		else
//		{
//			pWarning("Reading a NOT ALIVE change " << endl);
//
//		}
//		if(!isCacheRead(sn,wGuid))
//		{
//			ReadElement_t r_elem;
//			r_elem.seqNum = sn;
//			r_elem.writerGuid = wGuid;
//			readElements.push_back(r_elem);
//		}
//		return true;
//	}
//	else
//	{
//		pWarning("Change not found ");
//		return false;
//	}
//}
//
//bool Subscriber::readLastAdded(void* data_ptr)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//		CacheChange_t* change;
//		if(mp_Reader->m_reader_cache.get_last_added_cache(&change))
//		{
//			ReadElement_t rElem;
//			rElem.seqNum = change->sequenceNumber;
//			rElem.writerGuid = change->writerGUID;
//			readElements.push_back(rElem);
//
//			if(change->kind == ALIVE)
//			{
//				if(!mp_type->deserialize(&change->serializedPayload,data_ptr))
//				{
//					pWarning("Subscriber:Deserialization returns false"<<endl);
//					return false;
//				}
//			}
//			else
//			{
//				pWarning("Reading a NOT ALIVE change "<<endl);
//			}
//
//			return true;
//		}
//		else
//			pWarning("Problem reading last addded"<<endl);
//	}
//	pWarning("Not read anything. Reader Cache empty" << endl);
//	return false;
//}
//
//
//bool Subscriber::readAllUnreadCache(std::vector<void*>* data_vec)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//		if(mp_Reader->m_reader_cache.m_changes.size() == readElements.size())
//			return false; //No elements unread
//		SequenceNumber_t minSeqNum;
//		GUID_t minSeqNumGuid;
//		SEQUENCENUMBER_UNKOWN(minSeqNum);
//
//		bool noneRead = false;
//		bool read_this = false;
//		if(readElements.empty())
//			noneRead = true;
//		for(std::vector<CacheChange_t*>::iterator it=mp_Reader->m_reader_cache.m_changes.begin();it!=mp_Reader->m_reader_cache.m_changes.end();++it)
//		{
//			read_this = false;
//			if(noneRead)
//				read_this = true;
//			else //CHEK IF THE ELEMENT WAS ALREADY READ
//			{
//				if(!isCacheRead((*it)->sequenceNumber,(*it)->writerGUID))
//					read_this=true;
//			}
//			if(read_this)
//			{
//				ReadElement_t rElem;
//				rElem.seqNum = minSeqNum;
//				rElem.writerGuid = minSeqNumGuid;
//				readElements.push_back(rElem);
//				CacheChange_t* ch;
//				mp_Reader->m_reader_cache.get_change(minSeqNum,minSeqNumGuid,&ch);
//				if(ch->kind == ALIVE)
//				{
//					void * data_ptr = malloc(mp_type->m_typeSize);
//					if(!mp_type->deserialize(&ch->serializedPayload,data_ptr))
//					{
//						pWarning("Subscriber:Deserialization returns false"<<endl);
//						return false;
//					}
//					data_vec->push_back(data_ptr);
//				}
//				else
//					{pWarning("Cache with NOT ALIVE" << endl);}
//
//			}
//		}
//	}
//	pWarning("No elements in history ");
//
//	return false;
//}
//
//bool Subscriber::readMinSeqCache(void* data_ptr,SequenceNumber_t* minSeqNum, GUID_t* minSeqNumGuid)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//		mp_Reader->m_reader_cache.get_seq_num_min(minSeqNum,minSeqNumGuid);
//		ReadElement_t rElem;
//		rElem.seqNum = *minSeqNum;
//		rElem.writerGuid = *minSeqNumGuid;
//		if(!isCacheRead(*minSeqNum,*minSeqNumGuid))
//			readElements.push_back(rElem);
//		CacheChange_t* ch;
//		if(mp_Reader->m_reader_cache.get_change(*minSeqNum,*minSeqNumGuid,&ch))
//		{
//			if(ch->kind == ALIVE)
//			{
//
//				//boost::posix_time::ptime t1,t2;
//				//t1 = boost::posix_time::microsec_clock::local_time();
//				if(!mp_type->deserialize(&ch->serializedPayload,data_ptr))
//				{
//					pWarning("Subscriber:Deserialization returns false"<<endl);
//					return false;
//				}
//				//t2 = boost::posix_time::microsec_clock::local_time();
//				//cout<< "TIME total deserialize operation: " <<(t2-t1).total_microseconds()<< endl;
//			}
//			else
//			{pWarning("Cache with NOT ALIVE" << endl)}
//			return true;
//		}
//		else
//		{
//			pWarning("Min element not found")
//					return false;
//		}
//	}
//	pWarning("No elements in history " << endl);
//	return false;
//}
//
//bool Subscriber::readAllCache(std::vector<void*>* data_vec)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//
//		std::vector<ReadElement_t> readElem2;
//		for(std::vector<CacheChange_t*>::iterator it=mp_Reader->m_reader_cache.m_changes.begin();
//				it!=mp_Reader->m_reader_cache.m_changes.end();++it)
//		{
//			ReadElement_t r_elem;
//			r_elem.seqNum = (*it)->sequenceNumber;
//			r_elem.writerGuid = (*it)->writerGUID;
//			readElem2.push_back(r_elem);
//			if((*it)->kind==ALIVE)
//			{
//				void * data_ptr = malloc(mp_type->m_typeSize);
//				if(!mp_type->deserialize(&(*it)->serializedPayload,data_ptr))
//				{
//					pWarning("Subscriber:Deserialization returns false"<<endl);
//					return false;
//				}
//
//				data_vec->push_back(data_ptr);
//			}
//			else
//			{pWarning("Cache with NOT ALIVE" << endl);}
//		}
//		readElements = readElem2;
//		return true;
//	}
//	pWarning("No elements in history " << endl);
//	return false;
//}
//
//
//bool Subscriber::takeAllCache(std::vector<void*>* data_vec)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(readAllCache(data_vec))
//	{
//		mp_Reader->m_reader_cache.remove_all_changes();
//		readElements.clear();
//		return true;
//	}
//	pWarning("Not all cache read " << endl);
//
//	return false;
//}
//
//
//
//bool Subscriber::takeMinSeqCache(void* data_ptr)
//{
//	boost::lock_guard<HistoryCache> guard(mp_Reader->m_reader_cache);
//	if(!mp_Reader->m_reader_cache.m_changes.empty())
//	{
//		SequenceNumber_t seq;
//		GUID_t guid;
//		mp_Reader->m_reader_cache.get_seq_num_min(&seq,&guid);
//
//		ReadElement_t rElem;
//		rElem.seqNum = seq;
//		rElem.writerGuid = guid;
//		if(!isCacheRead(seq,guid))
//			readElements.push_back(rElem);
//		CacheChange_t* change = NULL;
//		uint16_t ch_number;
//		if(mp_Reader->m_reader_cache.get_change(seq,guid,&change,&ch_number))
//		{
//			if(change->kind == ALIVE)
//			{
//				if(!mp_type->deserialize(&change->serializedPayload,data_ptr))
//				{
//					pWarning("Subscriber:Deserialization returns false"<<endl);
//					return false;
//				}
//			}
//			else
//			{
//				pWarning("Cache with NOT ALIVE" << endl)
//			}
//			mp_Reader->m_reader_cache.release_Cache(change);
//			mp_Reader->m_reader_cache.m_changes.erase(mp_Reader->m_reader_cache.m_changes.begin()+ch_number);
//			removeSeqFromRead(seq,guid);
//			return true;
//		}
//		else
//		{
//			pWarning("Min Element not found"<<endl);
//			return false;
//		}
//	}
//	pWarning("No elements in history " << endl);
//	return false;
//}

bool Subscriber::minSeqRead(SequenceNumber_t* sn,GUID_t* guid,std::vector<ReadElement_t>::iterator* min_it)
{
	if(!readElements.empty())
	{
		std::vector<ReadElement_t>::iterator it;
		ReadElement_t minRead;
		minRead.seqNum.high = -1;
		for(it=readElements.begin();it!=readElements.end();++it)
		{
			if(minRead.seqNum.high == -1 ||
				minRead.seqNum.to64long() > it->seqNum.to64long())
			{
				minRead = *it;
				*min_it = it;
			}
		}
		*sn = minRead.seqNum;
		*guid = minRead.writerGuid;
		return true;
	}
	pWarning("No read elements " << endl);

	return false;
}

bool Subscriber::readNextData(void* data)
{
	return this->mp_Reader->readNextCacheChange(data);
}

bool Subscriber::takeNextData(void* data) {
	return this->mp_Reader->takeNextCacheChange(data);
}

bool Subscriber::removeSeqFromRead(SequenceNumber_t& sn,GUID_t& guid)
{
	if(!readElements.empty())
	{
		for(std::vector<ReadElement_t>::iterator it=readElements.begin();it!=readElements.end();++it)
		{
			if(it->seqNum.to64long()==sn.to64long()&&
					it->writerGuid == guid)
			{
				readElements.erase(it);
				return true;
			}
		}
		return false;
	}
	pWarning("No read elements " << endl);

	return false;
}

bool Subscriber::addWriterProxy(Locator_t& loc, GUID_t& guid)
{
	if(mp_Reader->m_stateType==STATELESS)
	{
		pError("StatelessReader cannot have writerProxy"<<endl);
		return false;
	}
	else if(mp_Reader->m_stateType==STATEFUL)
	{
		WriterProxy_t WL;
		WL.unicastLocatorList.push_back(loc);
		WL.remoteWriterGuid = guid;
		pDebugInfo("Adding WriterProxy at: "<< loc.to_IP4_string()<<":"<< loc.port<< endl);
		((StatefulReader*)mp_Reader)->matched_writer_add(&WL);
		return true;
	}
	return false;
}

//bool Subscriber::updateParameters(const SubscriberAttributes& param)
//{
//
//	return true;
//}


} /* namespace dds */
} /* namespace eprosima */


