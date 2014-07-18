/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapePublisher.cpp
 */

#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"


ShapePublisher::ShapePublisher(Participant* par):
    mp_pub(NULL),
    mp_participant(par),
    m_mutex(QMutex::Recursive),
    isInitialized(false)
{
	// TODO Auto-generated constructor stub

}

ShapePublisher::~ShapePublisher()
{
	// TODO Auto-generated destructor stub
    if(isInitialized)
    {
//        mp_pub->dispose((void*)&this->m_shape.m_mainShape);
//        mp_pub->unregister((void*)&this->m_shape.m_mainShape);
        mp_pub->dispose_and_unregister((void*)&this->m_shape.m_mainShape);
        DomainParticipant::removePublisher(this->mp_participant,mp_pub);
    }
}

bool ShapePublisher::initPublisher()
{
    mp_pub = DomainParticipant::createPublisher(mp_participant,m_attributes,(PublisherListener*)this);
    if(mp_pub !=NULL)
    {
        isInitialized = true;
        return true;
    }
    return false;
}

void ShapePublisher::write()
{
    if(mp_pub !=NULL)
    {
        mp_pub->write((void*)&this->m_shape.m_mainShape);
        //cout << "Trying to lock ShapePub: "<<std::flush;
        m_mutex.lock();
     //   cout << " OK "<<std::flush;
        m_drawShape = m_shape;
        m_mutex.unlock();
       // cout << " UNLOCKED ShapePub"<<endl;
    }
}

void ShapePublisher::onPublicationMatched(MatchingInfo info)
{
    if(info.status == MATCHED_MATCHING)
        cout << "Publisher MATCHES Sub: "<< info.remoteEndpointGuid << "*****************************"<<endl;
    else if(info.status == REMOVED_MATCHING)
        cout << "Publisher REMOVED Sub " << info.remoteEndpointGuid << "*****************************"<<endl;
}


