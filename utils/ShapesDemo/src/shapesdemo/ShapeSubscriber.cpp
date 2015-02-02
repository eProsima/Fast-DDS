/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapeSubscriber.cpp
 *
 */

#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"
#include "eprosimashapesdemo/qt/ContentFilterSelector.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/Domain.h"

ShapeSubscriber::ShapeSubscriber(Participant* par):
    mp_sub(nullptr),
    mp_participant(par),
    hasReceived(false),
    m_mutex(QMutex::Recursive),
    mp_contentFilter(nullptr)
{
    // TODO Auto-generated constructor stub

}

ShapeSubscriber::~ShapeSubscriber() {
    // TODO Auto-generated destructor stub
    Domain::removeSubscriber(mp_sub);
    if(mp_contentFilter!=nullptr)
        delete(mp_contentFilter);
}

bool ShapeSubscriber::initSubscriber()
{
    mp_sub = Domain::createSubscriber(mp_participant,m_attributes,(SubscriberListener*)this);
    if(mp_sub !=nullptr)
        return true;
    return false;
}




void ShapeSubscriber::onNewDataMessage(Subscriber *sub)
{
    // cout << "New DATA Message "<<endl;
    Shape shape;
    shape.m_type = this->m_shapeType;
    SampleInfo_t info;
    while(mp_sub->takeNextData((void*)&shape,&info))
    {
        // shape.m_x += 5;
        //cout << "Shape of type: "<< shape.m_type << "RECEIVED"<<endl;
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



void ShapeSubscriber::onSubscriptionMatched(Subscriber *sub, MatchingInfo& info)
{
    if(info.status ==MATCHED_MATCHING)
    {
        cout << "Subscriber  in topic" << m_attributes.topic.getTopicName() << " MATCHES Pub: " << info.remoteEndpointGuid <<"*****************************"<<endl;
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
        cout << "Subscriber in topic" << m_attributes.topic.getTopicName() << " REMOVES Pub: " << info.remoteEndpointGuid <<"*****************************"<<endl;
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

