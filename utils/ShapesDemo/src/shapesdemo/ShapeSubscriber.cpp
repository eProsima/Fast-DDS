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




void ShapeSubscriber::onNewDataMessage()
{
    // cout << "New DATA Message "<<endl;
    Shape shape;
    shape.m_type = this->m_shapeType;
    SampleInfo_t info;
    while(mp_sub->takeNextData((void*)&shape,&info))
    {
        // shape.m_x += 5;
        shape.m_time = info.sourceTimestamp;
        shape.m_writerGuid = info.writerGUID;
        shape.m_strength = info.ownershipStrength;
        QMutexLocker locck(&this->m_mutex);
        if(info.sampleKind == ALIVE)
        {
            hasReceived = true;
            m_shapeHistory.addToHistory(shape);
        }
        else
        {
            cout << "NOT ALIVE DATA"<<endl;
            //GET THE COLOR:
            SD_COLOR color = getColorFromInstanceHandle(info.iHandle);
            if(info.sampleKind == NOT_ALIVE_DISPOSED)
            {
                m_shapeHistory.dispose(color);
            }
            else
            {
                m_shapeHistory.unregister(color);
            }
        }
    }
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
        m_shapeHistory.removedOwner(info.remoteEndpointGuid);
        m_mutex.unlock();
    }
}

void ShapeSubscriber::adjustContentFilter(ShapeFilter &filter)
{
    m_mutex.lock();
    m_shapeHistory.adjustContentFilter(filter);
    m_mutex.unlock();
}

//void ShapeSubscriber::removeSamplesFromWriter(GUID_t)

