/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapeSubscriber.cpp
 *
 */

#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"

#include "eprosimartps/utils/TimeConversion.h"

ShapeSubscriber::ShapeSubscriber(Participant* par):
    mp_sub(NULL),
    mp_participant(par),
    hasReceived(false),
    m_mutex(QMutex::Recursive)
{
    // TODO Auto-generated constructor stub

}

ShapeSubscriber::~ShapeSubscriber() {
    // TODO Auto-generated destructor stub
    DomainParticipant::removeSubscriber(this->mp_participant,mp_sub);
}

bool ShapeSubscriber::initSubscriber()
{
    mp_sub = DomainParticipant::createSubscriber(mp_participant,m_attributes,(SubscriberListener*)this);
    if(mp_sub !=NULL)
        return true;
    return false;
}


inline bool compareGUID(GUID_t& g1, GUID_t& g2)
{
    for(uint8_t i =0;i<16;++i)
    {
        if(i<12)
        {
            if(g1.guidPrefix.value[i]<g2.guidPrefix.value[i])
                return true;
            else if(g1.guidPrefix.value[i]>g2.guidPrefix.value[i])
                return false;
        }
        else
        {
            if(g1.entityId.value[i-12]<g2.entityId.value[i-12])
                return true;
            else if(g1.entityId.value[i-12]>g2.entityId.value[i-12])
                return false;
        }
    }
    return false;
}


void ShapeSubscriber::onNewDataMessage()
{
    // cout << "New DATA Message "<<endl;
    ShapeType shape;
    SampleInfo_t info;
    while(mp_sub->readNextData((void*)&shape,&info))
    {
        // shape.m_x += 5;
        shape.m_time = info.sourceTimestamp;
        shape.m_writerGuid = info.writerGUID;
        shape.m_strength = info.ownershipStrength;
        // cout << "DATA size: " << shape.m_size << " Strength: "<< shape.m_strength << endl;
        if(info.sampleKind == ALIVE)
        {
            //  cout << "ALive sample, ";
            hasReceived = true;
            bool found = false;
            m_mutex.lock();
            for(std::vector<std::list<ShapeType>>::iterator it = m_shape.m_shapeHistory.begin();
                it!=m_shape.m_shapeHistory.end();++it)
            {
                if(it->begin()->getColor() == shape.getColor())
                {
                    //  cout << " found with the same color"<<endl;
                    found = true;
                    // cout << "Time DIFF: "<<Time_tAbsDiff2Millisec(it->front().m_time,info.sourceTimestamp)<<endl;
                    // cout << "Minimum separation: "<< Time_t2MilliSec(m_attributes.qos.m_timeBasedFilter.minimum_separation)<<endl;
                    // cout << "Pass filter: "<<  passFilter(&shape) << endl;
                    if(TimeConv::Time_tAbsDiff2DoubleMillisec(it->front().m_time,info.sourceTimestamp)>=TimeConv::Time_t2MilliSecondsDouble(m_attributes.qos.m_timeBasedFilter.minimum_separation) &&
                            passFilter(&shape))
                    {
                        if(this->m_attributes.qos.m_ownership.kind == SHARED_OWNERSHIP_QOS)
                        {
                            it->push_front(shape);
                            if(it->size() > m_attributes.topic.historyQos.depth)
                            {
                                it->pop_back();
                            }
                        }
                        else
                        {
                            // cout << "SHARED OWNERSHIP "<<endl;
                            // cout << "IT STRENGTH: "<< it->front().m_strength << endl;
                            if( shape.m_strength > it->front().m_strength &&
                                    shape.m_writerGuid != it->front().m_writerGuid)
                            {
                                // cout << "MORE STRENGTH; DIFFERENT WRITER "<<endl;
                                it->clear();
                                it->push_front(shape);
                            }
                            else if(shape.m_strength == it->front().m_strength &&
                                    shape.m_writerGuid == it->front().m_writerGuid)
                            {
                                //  cout << "SAME STRENGTH; SAME WRITER "<<endl;
                                it->push_front(shape);
                                if(it->size() > m_attributes.topic.historyQos.depth)
                                {
                                    it->pop_back();
                                }
                            }
                            else if(shape.m_strength == it->front().m_strength &&
                                    compareGUID(shape.m_writerGuid,it->front().m_writerGuid))
                            {
                                //  cout << "SAME STRENGTH; DIFFERENT WRITER (<) "<<endl;l
                                it->clear();
                                it->push_front(shape);
                            }
                        }
                    }
                }
            }
            if(!found && passFilter(&shape))
            {
                m_shape.m_shapeHistory.push_back(std::list<ShapeType>(1,shape));
            }

            //cout << "OK "<<std::flush;
            m_drawShape = m_shape;
            m_mutex.unlock();
            //cout << " UNLOCKED SHAPESub"<<endl;
        }
        else
        {
            cout << "NOT ALIVE DATA"<<endl;
            for(std::vector<std::list<ShapeType>>::iterator it = m_shape.m_shapeHistory.begin();
                it!=m_shape.m_shapeHistory.end();++it)
            {
                if(it->begin()->getColor() == shape.getColor())
                {
                    m_shape.m_shapeHistory.erase(it);
                    break;
                }
            }
        }
    }
}


bool ShapeSubscriber::passFilter(ShapeType* shape)
{
    if(!m_filter.m_useFilter)
        return true;
    else
    {
        if(shape->m_x < m_filter.m_maxX &&
                shape->m_x > m_filter.m_minX &&
                shape->m_y < m_filter.m_maxY &&
                shape->m_y > m_filter.m_minY)
        {
            // cout << "FILTER PASSED"<<endl;
            return true;
        }
    }
    return false;
}

void ShapeSubscriber::onSubscriptionMatched(MatchingInfo info)
{
    if(info.status ==MATCHED_MATCHING)
    {
        cout << "Subscriber MATCHES Pub: " << info.remoteEndpointGuid <<"*****************************"<<endl;
        bool found = false;
        for(std::vector<GUID_t>::iterator it = m_remoteWriters.begin();
            it!=m_remoteWriters.end();++it)
        {
            if(*it==info.remoteEndpointGuid)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            m_remoteWriters.push_back(info.remoteEndpointGuid);
        }

    }
    else if(info.status == REMOVED_MATCHING)
    {
       cout << "Subscriber REMOVED Pub: " << info.remoteEndpointGuid <<"*****************************"<<endl;
        m_mutex.lock();
        for(std::vector<std::list<ShapeType>>::iterator it = m_shape.m_shapeHistory.begin();
            it!=m_shape.m_shapeHistory.end();++it)
        {
            cout << it->front().m_writerGuid << endl;
            cout << info.remoteEndpointGuid << endl;
            if(it->front().m_writerGuid == info.remoteEndpointGuid)
            {
                cout << "FOUND, DELETING"<<endl;
                m_shape.m_shapeHistory.erase(it);
                break;
            }
        }
        m_mutex.unlock();
    }
}

//void ShapeSubscriber::removeSamplesFromWriter(GUID_t)

