/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapePublisher.cpp
 */

#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"


ShapePublisher::ShapePublisher(RTPSParticipant* par):
    mp_pub(NULL),
    mp_RTPSParticipant(par),
    //m_mutex(QMutex::Recursive),
    isInitialized(false),
    hasWritten(false)
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
        mp_pub->dispose_and_unregister((void*)&this->m_shape);
        DomainRTPSParticipant::removePublisher(this->mp_RTPSParticipant,mp_pub);
    }
}

bool ShapePublisher::initPublisher()
{
    mp_pub = DomainRTPSParticipant::createPublisher(mp_RTPSParticipant,m_attributes,(PublisherListener*)this);
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
        mp_pub->write((void*)&this->m_shape);
        //cout << "Trying to lock ShapePub: "<<std::flush;
        //m_mutex.lock();
        //cout << " OK "<<std::flush;
        hasWritten = true;
        //m_mutex.unlock();

       // cout << " UNLOCKED ShapePub"<<endl;
    }
}

void ShapePublisher::onPublicationMatched(Publisher* pub,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
        cout << "Publisher MATCHES Sub: "<< info.remoteEndpointGuid << "*****************************"<<endl;
    else if(info.status == REMOVED_MATCHING)
        cout << "Publisher REMOVED Sub " << info.remoteEndpointGuid << "*****************************"<<endl;
}


