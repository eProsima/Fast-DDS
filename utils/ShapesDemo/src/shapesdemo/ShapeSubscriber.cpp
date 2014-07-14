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
    cout << "New DATA Message "<<endl;
    ShapeType shape;
    SampleInfo_t info;
    while(mp_sub->readNextData((void*)&shape,&info))
    {
       // shape.m_x += 5;
        shape.m_time = info.sourceTimestamp;
        if(info.sampleKind == ALIVE)
        {
            hasReceived = true;
            bool found = false;
            for(std::vector<std::list<ShapeType>>::iterator it = m_shape.m_shapeHistory.begin();
                it!=m_shape.m_shapeHistory.end();++it)
            {
                if(it->begin()->getColor() == shape.getColor())
                {
                    if(Time_tAbsDiff2Millisec(it->front().m_time,info.sourceTimestamp)>Time_t2MilliSec(m_attributes.qos.m_timeBasedFilter.minimum_separation) &&
                            passFilter(&shape))
                    {
                        it->push_front(shape);
                        if(it->size() > m_attributes.topic.historyQos.depth)
                        {
                            it->pop_back();
                        }
                        found = true;
                    }
                }
            }
            if(!found)
            {
                m_shape.m_shapeHistory.push_back(std::list<ShapeType>(1,shape));
            }
            m_mutex.lock();
            //cout << "OK "<<std::flush;
            m_drawShape = m_shape;
            m_mutex.unlock();
            //cout << " UNLOCKED SHAPESub"<<endl;
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
            return true;
        }
    }
    return false;
}

void ShapeSubscriber::onSubscriptionMatched()
{
    cout << "SUBSCRIBED:*****************************"<<endl;
}

