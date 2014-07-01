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
    hasReceived(false)
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

    ShapeType shape;
    SampleInfo_t info;
    while(mp_sub->readNextData((void*)&shape,&info))
    {
       // shape.m_x += 5;
        if(info.sampleKind == ALIVE)
        {
            if(m_shape.m_history.size() < m_attributes.topic.historyQos.depth -1)
            {

            }
            else
            {
                m_shape.m_history.pop_back();
            }
            if(!hasReceived)
            {
                hasReceived = true;
            }
            else
            {
                m_shape.m_history.push_front(m_shape.m_mainShape);
            }
            m_shape.m_mainShape = shape;
        }
    }
}

void ShapeSubscriber::onSubscriptionMatched()
{
    cout << "SUBSCRIBED:*****************************"<<endl;
}
