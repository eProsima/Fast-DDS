/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.cpp
 *
 */

#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>
#include <fastrtps/utils/RTPSLog.h>
#include "../participant/RTPSParticipantImpl.h"

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

#include <cassert>

#define IDSTRING "(ID:"<< boost::this_thread::get_id() <<") "<<

using namespace eprosima::fastrtps::rtps;


static const char* const CLASS_NAME = "StatefulReader";

StatefulReader::~StatefulReader()
{
    const char* const METHOD_NAME = "~StatefulReader";
    logInfo(RTPS_READER,"StatefulReader destructor.";);
    for(std::vector<WriterProxy*>::iterator it = matched_writers.begin();
            it!=matched_writers.end();++it)
    {
        delete(*it);
    }
}



StatefulReader::StatefulReader(RTPSParticipantImpl* pimpl,GUID_t& guid,
        ReaderAttributes& att,ReaderHistory* hist,ReaderListener* listen):
    RTPSReader(pimpl,guid,att,hist, listen),
    m_times(att.times)
{

}


bool StatefulReader::matched_writer_add(RemoteWriterAttributes& wdata)
{
    const char* const METHOD_NAME = "matched_writer_add";
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();
            it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == wdata.guid)
        {
            logInfo(RTPS_READER,"Attempting to add existing writer");
            return false;
        }
    }
    WriterProxy* wp = new WriterProxy(wdata,m_times.heartbeatResponseDelay,this);
    matched_writers.push_back(wp);
    logInfo(RTPS_READER,"Writer Proxy " <<wp->m_att.guid <<" added to " <<m_guid.entityId);
    return true;
}

bool StatefulReader::matched_writer_remove(RemoteWriterAttributes& wdata)
{
    const char* const METHOD_NAME = "matched_writer_remove";
    WriterProxy *wproxy = nullptr;
    boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == wdata.guid)
        {
            logInfo(RTPS_READER,"Writer Proxy removed: " <<(*it)->m_att.guid);
            wproxy = *it;
            matched_writers.erase(it);
            break;
        }
    }

    lock.unlock();

    if(wproxy != nullptr)
    {
        delete wproxy;
        return true;
    }

    logInfo(RTPS_READER,"Writer Proxy " << wdata.guid << " doesn't exist in reader "<<this->getGuid().entityId);
    return false;
}

bool StatefulReader::matched_writer_remove(RemoteWriterAttributes& wdata,bool deleteWP)
{
    const char* const METHOD_NAME = "matched_writer_remove";
    WriterProxy *wproxy = nullptr;
    boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == wdata.guid)
        {
            logInfo(RTPS_READER,"Writer Proxy removed: " <<(*it)->m_att.guid);
            wproxy = *it;
            matched_writers.erase(it);
            break;
        }
    }

    lock.unlock();

    if(wproxy != nullptr && deleteWP)
    {
        delete(wproxy);
        return true;
    }

    logInfo(RTPS_READER,"Writer Proxy " << wdata.guid << " doesn't exist in reader "<<this->getGuid().entityId);
    return false;
}

bool StatefulReader::matched_writer_is_matched(RemoteWriterAttributes& wdata)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    for(std::vector<WriterProxy*>::iterator it=matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == wdata.guid)
        {
            return true;
        }
    }
    return false;
}


bool StatefulReader::matched_writer_lookup(GUID_t& writerGUID, WriterProxy** WP)
{
    const char* const METHOD_NAME = "matched_writer_lookup";
    assert(WP);

    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    for(std::vector<WriterProxy*>::iterator it = matched_writers.begin(); it != matched_writers.end(); ++it)
    {
        if((*it)->m_att.guid == writerGUID)
        {
            *WP = *it;
            logInfo(RTPS_READER,this->getGuid().entityId<<" FINDS writerProxy "<< writerGUID<<" from "<< matched_writers.size());
            return true;
        }
    }
    logInfo(RTPS_READER,this->getGuid().entityId<<" NOT FINDS writerProxy "<< writerGUID<<" from "<< matched_writers.size());
    return false;
}

bool StatefulReader::processDataMsg(CacheChange_t *change)
{
    const char* const METHOD_NAME = "processDataMsg";
    WriterProxy *pWP = nullptr;

    assert(change);

    boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(change->writerGUID, &pWP))
    {
        logInfo(RTPS_MSG_IN,IDSTRING"Trying to add change " << change->sequenceNumber <<" TO reader: "<< getGuid().entityId,C_BLUE);

        CacheChange_t* change_to_add;

        if(reserveCache(&change_to_add)) //Reserve a new cache from the corresponding cache pool
        { 
            if (!change_to_add->copy(change))
            {
                logWarning(RTPS_MSG_IN,IDSTRING"Problem copying CacheChange, received data is: " << change->serializedPayload.length
                        << " bytes and max size in reader " << getGuid().entityId << " is " << change_to_add->serializedPayload.max_size, C_BLUE);
                releaseCache(change_to_add);
                return false;
            }
        }
        else
        {
            logError(RTPS_MSG_IN,IDSTRING"Problem reserving CacheChange in reader: " << getGuid().entityId, C_BLUE);
            return false;
        }

        if(pWP != nullptr)
        {
            pWP->assertLiveliness(); //Asser liveliness since you have received a DATA MESSAGE.
        }

        if(!change_received(change_to_add, pWP, lock))
        {
            logInfo(RTPS_MSG_IN,IDSTRING"MessageReceiver not add change "<<change_to_add->sequenceNumber, C_BLUE);
            releaseCache(change_to_add);

            if(pWP == nullptr && getGuid().entityId == c_EntityId_SPDPReader)
            {
                mp_RTPSParticipant->assertRemoteRTPSParticipantLiveliness(change->writerGUID.guidPrefix);
            }
        }
    }

    return true;
}

bool StatefulReader::processDataFragMsg(CacheChange_t *change, uint32_t sampleSize, uint32_t fragmentStartingNum)
{
	return false;
}

bool StatefulReader::processHeartbeatMsg(GUID_t &writerGUID, uint32_t hbCount, SequenceNumber_t &firstSN,
            SequenceNumber_t &lastSN, bool finalFlag, bool livelinessFlag)
{
    WriterProxy *pWP = nullptr;

    boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(writerGUID, &pWP, false))
    {
        boost::lock_guard<boost::recursive_mutex> guardWriterProxy(*pWP->getMutex());

        if(pWP->m_lastHeartbeatCount < hbCount)
        {
            pWP->m_lastHeartbeatCount = hbCount;
            pWP->lost_changes_update(firstSN);
            pWP->missing_changes_update(lastSN);
            pWP->m_heartbeatFinalFlag = finalFlag;
            //Analyze wheter a acknack message is needed:

            if(!finalFlag)
            {
                pWP->mp_heartbeatResponse->restart_timer();
            }
            else if(finalFlag && !livelinessFlag)
            {
                if(!pWP->m_isMissingChangesEmpty)
                    pWP->mp_heartbeatResponse->restart_timer();
            }

            //FIXME: livelinessFlag
            if(livelinessFlag )//TODOG && WP->m_att->m_qos.m_liveliness.kind == MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                pWP->assertLiveliness();
            }
        }
    }

    return true;
}

bool StatefulReader::processGapMsg(GUID_t &writerGUID, SequenceNumber_t &gapStart, SequenceNumberSet_t &gapList)
{
    WriterProxy *pWP = nullptr;

    boost::unique_lock<boost::recursive_mutex> lock(*mp_mutex);

    if(acceptMsgFrom(writerGUID, &pWP, false))
    {
        boost::lock_guard<boost::recursive_mutex> guardWriterProxy(*pWP->getMutex());
        SequenceNumber_t auxSN;
        SequenceNumber_t finalSN = gapList.base -1;
        for(auxSN = gapStart; auxSN<=finalSN;auxSN++)
            pWP->irrelevant_change_set(auxSN);

        for(std::vector<SequenceNumber_t>::iterator it=gapList.get_begin();it!=gapList.get_end();++it)
            pWP->irrelevant_change_set((*it));
    }

    return true;
}

bool StatefulReader::acceptMsgFrom(GUID_t &writerId, WriterProxy **wp, bool checkTrusted)
{
    assert(wp != nullptr);

    if(checkTrusted && writerId.entityId == this->m_trustedWriterEntityId)
        return true;

    for(std::vector<WriterProxy*>::iterator it = this->matched_writers.begin();
            it!=matched_writers.end();++it)
    {
        if((*it)->m_att.guid == writerId)
        {
            *wp = *it;
            return true;
        }
    }

    return false;
}

bool StatefulReader::change_removed_by_history(CacheChange_t* a_change,WriterProxy* wp)
{
    const char* const METHOD_NAME = "change_removed_by_history";
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    if(wp!=nullptr || matched_writer_lookup(a_change->writerGUID,&wp))
    {
        boost::lock_guard<boost::recursive_mutex> guardWriterProxy(*wp->getMutex());
        std::vector<int> to_remove;
        bool continuous_removal = true;
        for(size_t i = 0;i<wp->m_changesFromW.size();++i)
        {
            if(a_change->sequenceNumber == wp->m_changesFromW.at(i).seqNum)
            {
                wp->m_changesFromW.at(i).notValid();
                if(continuous_removal)
                {
                    wp->m_lastRemovedSeqNum = wp->m_changesFromW.at(i).seqNum;
				    wp->m_hasMinAvailableSeqNumChanged = true;
                    to_remove.push_back((int)i);
                }
                break;
            }
            if(!wp->m_changesFromW.at(i).isValid()
                    && (wp->m_changesFromW.at(i).status == RECEIVED || wp->m_changesFromW.at(i).status == LOST)
                    && continuous_removal)
            {
                wp->m_lastRemovedSeqNum = wp->m_changesFromW.at(i).seqNum;
                wp->m_hasMinAvailableSeqNumChanged = true;
                to_remove.push_back((int)i);
                continue;
            }
            continuous_removal = false;
        }
        for(std::vector<int>::reverse_iterator it = to_remove.rbegin();
                it!=to_remove.rend();++it)
        {
            wp->m_changesFromW.erase(wp->m_changesFromW.begin()+*it);
        }
        return true;
    }
    else
    {
        logError(RTPS_READER," You should always find the WP associated with a change, something is very wrong");
    }
    return false;
}

bool StatefulReader::change_received(CacheChange_t* a_change, WriterProxy* prox, boost::unique_lock<boost::recursive_mutex> &lock)
{
    const char* const METHOD_NAME = "change_received";

    //First look for WriterProxy in case is not provided
    if(prox == nullptr)
    {
        if(!this->matched_writer_lookup(a_change->writerGUID,&prox))
        {
            logInfo(RTPS_READER, "Writer Proxy " << a_change->writerGUID <<" not matched to this Reader "<< m_guid.entityId);
            return false;
        }
    }

    boost::unique_lock<boost::recursive_mutex> writerProxyLock(*prox->getMutex());

    //WITH THE WRITERPROXY FOUND:
    //Check if we can add it
    if(a_change->sequenceNumber <= prox->m_lastRemovedSeqNum)
    {
        logInfo(RTPS_READER, "Change "<<a_change->sequenceNumber<< " <= than last Removed Seq Number "<< prox->m_lastRemovedSeqNum);
        return false;
    }
    SequenceNumber_t maxSeq;
    prox->available_changes_max(&maxSeq);
    if(a_change->sequenceNumber <= maxSeq)
    {
        logInfo(RTPS_READER, "Change "<<a_change->sequenceNumber<< " <= than max available Seqnum "<<maxSeq);
        return false;
    }
    if(this->mp_history->received_change(a_change))
    {
        if(prox->received_change_set(a_change))
        {
            if(getListener()!=nullptr)
            {
                SequenceNumber_t maxSeqNumAvailable;
                prox->available_changes_max(&maxSeqNumAvailable);
                GUID_t proxGUID = prox->m_att.guid;
                writerProxyLock.unlock();

                if(a_change->sequenceNumber == maxSeqNumAvailable)
                {
                    lock.unlock();
                    getListener()->onNewCacheChangeAdded((RTPSReader*)this,a_change);
                }
                else if(a_change->sequenceNumber < maxSeqNumAvailable)
                {
                    SequenceNumber_t notifySeqNum = a_change->sequenceNumber + 1;

                    lock.unlock();
                    getListener()->onNewCacheChangeAdded((RTPSReader*)this,a_change);
                    lock.lock();

                    CacheChange_t* ch_to_give = nullptr;
                    //TODO Intentar optimizar esto para que no haya que recorrer la lista de cambios cada vez
                    while(notifySeqNum <= maxSeqNumAvailable)
                    {
                        ch_to_give = nullptr;
                        if(mp_history->get_change(notifySeqNum, proxGUID, &ch_to_give))
                        {
                            if(!ch_to_give->isRead)
                            {
                                lock.unlock();
                                getListener()->onNewCacheChangeAdded((RTPSReader*)this,ch_to_give);
                                lock.lock();
                            }
                        }
                        notifySeqNum++;
                    }
                }
                else
                {
                    //DO NOTHING; SOME CHANGES ARE MISSING
                }
            }
            //			if(a_change->sequenceNumber <= maxSeqNumAvailable)
            //			{
            //				if(getListener()!=nullptr) //TODO while del actual al maximo. y llamar al metodo, solo si no esta leido.
            //				{
            //					getListener()->onNewCacheChangeAdded((RTPSReader*)this,a_change);
            //				}
            //				mp_history->postSemaphore();
            //			}
            return true;
        }
    }
    return false;
}




//
bool StatefulReader::nextUntakenCache(CacheChange_t** change,WriterProxy** wpout)
{
    const char* const METHOD_NAME = "nextUntakenCache";
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    SequenceNumber_t minSeqNum = c_SequenceNumber_Unknown;
    SequenceNumber_t auxSeqNum;
    WriterProxy* wp = nullptr;
    bool available = false;
    logInfo(RTPS_READER,this->getGuid().entityId<<": looking through: "<< matched_writers.size() << " WriterProxies");
    for(std::vector<WriterProxy*>::iterator it = this->matched_writers.begin();it!=matched_writers.end();++it)
    {
        if((*it)->available_changes_min(&auxSeqNum))
        {
            //logUser("AVAILABLE MIN for writer: "<<(*it)->m_att.guid<< " : " << auxSeqNum);
            if(auxSeqNum > SequenceNumber_t(0, 0) && (minSeqNum > auxSeqNum || minSeqNum == c_SequenceNumber_Unknown))
            {
                available = true;
                minSeqNum = auxSeqNum;
                wp = *it;
            }
        }
    }
    //cout << "AVAILABLE? "<< available << endl;
    if(available && wp->get_change(minSeqNum,change))
    {
        if(wpout !=nullptr)
            *wpout = wp;
        return true;
        //		logInfo(RTPS_READER,this->getGuid().entityId<<": trying takeNextCacheChange: "<< min_change->sequenceNumber.to64long());
        //		if(min_change->kind == ALIVE)
        //			this->mp_type->deserialize(&min_change->serializedPayload,data);
        //		if(wp->removeChangesFromWriterUpTo(min_change->sequenceNumber))
        //		{
        //			if(info!=NULL)
        //			{
        //				info->sampleKind = min_change->kind;
        //				info->writerGUID = min_change->writerGUID;
        //				info->sourceTimestamp = min_change->sourceTimestamp;
        //				if(this->m_qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
        //					info->ownershipStrength = wp->m_data->m_qos.m_ownershipStrength.value;
        //				if(!min_change->isRead)
        //					m_reader_cache.decreaseUnreadCount();
        //				info->iHandle = min_change->instanceHandle;
        //			}
        //			if(!m_reader_cache.remove_change(min_change))
        //				logWarning(RTPS_READER,"Problem removing change " << min_change->sequenceNumber <<" from ReaderHistory");
        //			return true;
        //		}
    }
    return false;
}
//
bool StatefulReader::nextUnreadCache(CacheChange_t** change,WriterProxy** wpout)
{
    const char* const METHOD_NAME = "nextUnreadCache";
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    std::vector<CacheChange_t*> toremove;
    bool readok = false;
    for(std::vector<CacheChange_t*>::iterator it = mp_history->changesBegin();
            it!=mp_history->changesEnd();++it)
    {
        if((*it)->isRead)
            continue;
        WriterProxy* wp;
        if(this->matched_writer_lookup((*it)->writerGUID,&wp))
        {
            SequenceNumber_t seq;
            wp->available_changes_max(&seq);
            if(seq >= (*it)->sequenceNumber)
            {
                *change = *it;
                if(wpout !=nullptr)
                    *wpout = wp;
                return true;
                //				if((*it)->kind == ALIVE)
                //				{
                //					this->mp_type->deserialize(&(*it)->serializedPayload,data);
                //				}
                //				(*it)->isRead = true;
                //				if(info!=NULL)
                //				{
                //					info->sampleKind = (*it)->kind;
                //					info->writerGUID = (*it)->writerGUID;
                //					info->sourceTimestamp = (*it)->sourceTimestamp;
                //					info->iHandle = (*it)->instanceHandle;
                //					if(this->m_qos.m_ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
                //						info->ownershipStrength = wp->m_data->m_qos.m_ownershipStrength.value;
                //				}
                //				m_reader_cache.decreaseUnreadCount();
                //				logInfo(RTPS_READER,this->getGuid().entityId<<": reading change "<< (*it)->sequenceNumber.to64long());
                //				readok = true;
                //				break;
            }
        }
        else
        {
            toremove.push_back((*it));
        }
    }

    for(std::vector<CacheChange_t*>::iterator it = toremove.begin();
            it!=toremove.end();++it)
    {
        logWarning(RTPS_READER,"Removing change "<<(*it)->sequenceNumber << " from " << (*it)->writerGUID << " because is no longer paired");
        mp_history->remove_change(*it);
    }
    return readok;
}

//
//bool StatefulReader::acceptMsgFrom(GUID_t& writerId,WriterProxy** wp)
//{
//	if(this->m_acceptMessagesFromUnkownWriters)
//	{
//		for(std::vector<WriterProxy*>::iterator it = this->matched_writers.begin();
//				it!=matched_writers.end();++it)
//		{
//			if((*it)->m_data->m_guid == writerId)
//			{
//				if(wp!=NULL)
//					*wp = *it;
//				return true;
//			}
//		}
//	}
//	else
//	{
//		if(writerId.entityId == this->m_trustedWriterEntityId)
//			return true;
//	}
//	return false;
//}
//
bool StatefulReader::updateTimes(ReaderTimes& ti)
{
    boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    if(m_times.heartbeatResponseDelay != ti.heartbeatResponseDelay)
    {
        m_times = ti;
        for(std::vector<WriterProxy*>::iterator wit = this->matched_writers.begin();
                wit!=this->matched_writers.end();++wit)
        {
            (*wit)->mp_heartbeatResponse->update_interval(m_times.heartbeatResponseDelay);
        }
    }
    return true;
}
