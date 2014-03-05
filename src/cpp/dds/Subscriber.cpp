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

#include "eprosimartps/Subscriber.h"
#include "eprosimartps/RTPSReader.h"

namespace eprosima {
namespace dds {

Subscriber::Subscriber() {
	// TODO Auto-generated constructor stub

}

Subscriber::Subscriber(RTPSReader* Rin) {
	R = Rin;
	R->newMessageCallback = NULL;
	R->newMessageSemaphore = new boost::interprocess::interprocess_semaphore(0);
}


Subscriber::~Subscriber() {
	// TODO Auto-generated destructor stub
}


void Subscriber::blockUntilNewMessage(){
	R->newMessageSemaphore->wait();
}

void Subscriber::assignNewMessageCallback(void (*fun)()) {
	R->newMessageCallback = fun;
}


bool Subscriber::readAllUnread(std::vector<void*>* data_vec) {
	if(!R->reader_cache.changes.empty())
	{
		std::vector<CacheChange_t*>::iterator it;
		std::vector<SequenceNumber_t>::iterator sn_it;
		R->reader_cache.historyMutex.lock();
		bool is_read = false;
		bool found_one = false;
		for(it=R->reader_cache.changes.begin();it!=R->reader_cache.changes.end();it++)
		{
			is_read = false;
			for(sn_it=readCacheChanges.begin();sn_it!=readCacheChanges.end();sn_it++)
			{
				if(*sn_it == (*it)->sequenceNumber)
				{
					is_read = true;
					break;
				}
			}
			if(!is_read)
			{
				void* data = malloc(type.byte_size);
				type.deserialize(&(*it)->serializedPayload,data);
				data_vec->push_back(data);
				readCacheChanges.push_back((*it)->sequenceNumber);
				found_one = true;
			}
		}
		R->reader_cache.historyMutex.unlock();
		if(found_one)
			return true;
	}
	return false;
}

bool Subscriber::readSeqNum(SequenceNumber_t sn, void* data) {
	std::vector<CacheChange_t*>::iterator it;
	R->reader_cache.historyMutex.lock();
	for(it=R->reader_cache.changes.begin();it!=R->reader_cache.changes.end();it++)
	{
		if(sn == (*it)->sequenceNumber)
		{
			type.deserialize(&(*it)->serializedPayload,data);
			readCacheChanges.push_back((*it)->sequenceNumber);
			R->reader_cache.historyMutex.unlock();
				return true;
		}
	}
	R->reader_cache.historyMutex.unlock();
	return false;
}

bool Subscriber::readOlderUnread(void* data)
{
	if(!R->reader_cache.changes.empty())
	{
		R->reader_cache.historyMutex.lock();
		SequenceNumber_t minSeqNum;
		bool first = true;
		std::vector<CacheChange_t*>::iterator it;
		std::vector<SequenceNumber_t>::iterator sn_it;
		bool is_read = false;
		bool found_one = false;
		if(readCacheChanges.size()==0) // No element is read
		{
			minSeqNum = R->reader_cache.get_seq_num_min();
			found_one = true;
		}
		else
		{
			for(it=R->reader_cache.changes.begin();it!=R->reader_cache.changes.end();it++)
			{
				is_read = false;
				for(sn_it=readCacheChanges.begin();sn_it!=readCacheChanges.end();sn_it++)
				{
					if(*sn_it == (*it)->sequenceNumber)
					{
						is_read = true;
						break;
					}
				}
				if(!is_read)
				{
					if(first)
					{
						minSeqNum = (*it)->sequenceNumber;
						first = false;
					}
					if((*it)->sequenceNumber.to64long() < minSeqNum.to64long())
					{
						minSeqNum = (*it)->sequenceNumber;
					}
					found_one = true;
				}
			}
		}
		if(found_one)
		{
			readCacheChanges.push_back(minSeqNum);
			R->reader_cache.historyMutex.unlock();
			CacheChange_t* ch;
			R->reader_cache.get_change(minSeqNum,&ch);
			type.deserialize(&ch->serializedPayload,data);
			return true;
		}
		R->reader_cache.historyMutex.unlock();
	}
	return false;
}



bool Subscriber::takeOlderRead()
{
	if(!readCacheChanges.empty())
	{
		R->reader_cache.historyMutex.lock();
		std::vector<CacheChange_t*>::iterator cit;
		std::vector<SequenceNumber_t>::iterator sn_it;
		std::vector<SequenceNumber_t>::iterator min_it;
		SequenceNumber_t minSeqNum = readCacheChanges[0];
		min_it = readCacheChanges.begin();
		for(sn_it=readCacheChanges.begin()+1;sn_it!=readCacheChanges.end();sn_it++)
		{
			if(minSeqNum.to64long() > sn_it->to64long())
			{
				minSeqNum = *sn_it;
				min_it = sn_it;
			}
		}
		for(cit=R->reader_cache.changes.begin();cit!=R->reader_cache.changes.end();cit++)
		{
			if(minSeqNum == (*cit)->sequenceNumber)
			{
				if(R->reader_cache.remove_change(minSeqNum))
				{
					readCacheChanges.erase(min_it);
					R->reader_cache.historyMutex.unlock();
					return true;
				}

			}
		}
		R->reader_cache.historyMutex.unlock();
		return false;
	}
	return false;
}

bool Subscriber::takeAllRead()
{
	if(!readCacheChanges.empty())
	{
		R->reader_cache.historyMutex.lock();
		std::vector<CacheChange_t*>::iterator cit;
		std::vector<SequenceNumber_t>::iterator sn_it;
		bool removeAll = true;
		for(sn_it=readCacheChanges.begin()+1;sn_it!=readCacheChanges.end();sn_it++)
		{
			for(cit=R->reader_cache.changes.begin();cit!=R->reader_cache.changes.end();cit++)
			{
				if(*sn_it == (*cit)->sequenceNumber)
				{
					if(R->reader_cache.remove_change(*sn_it))
					{
						readCacheChanges.erase(sn_it);
					}
					else
						removeAll = false;

				}
			}
		}
		R->reader_cache.historyMutex.unlock();
		if(!removeAll)
			return false;
		else
			return true;
	}
	return false;
}



} /* namespace dds */
} /* namespace eprosima */


