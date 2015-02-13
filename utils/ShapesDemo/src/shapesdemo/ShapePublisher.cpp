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
#include "fastrtps/Domain.h"
#include "fastrtps/publisher/Publisher.h"


ShapePublisher::ShapePublisher(Participant* par):
    mp_pub(nullptr),
    mp_participant(par),
    m_mutex(QMutex::Recursive),
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
        Domain::removePublisher(mp_pub);
    }
}

bool ShapePublisher::initPublisher()
{
    mp_pub = Domain::createPublisher(mp_participant,m_attributes,(PublisherListener*)this);
    if(mp_pub !=nullptr)
    {
         isInitialized = true;
         return true;
    }
    return false;
}

void ShapePublisher::write()
{
    if(mp_pub !=nullptr)
    {
        mp_pub->write((void*)&this->m_shape);
        //cout << "Trying to lock ShapePub: "<<std::flush;
        m_mutex.lock();
     //   cout << " OK "<<std::flush;
        hasWritten = true;
        m_mutex.unlock();

       // cout << " UNLOCKED ShapePub"<<endl;
    }
}

void ShapePublisher::onPublicationMatched(Publisher *pub, MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
        cout << "Publisher  in topic " << m_attributes.topic.getTopicName() << " MATCHES Sub: "<< info.remoteEndpointGuid << "*****************************"<<endl;
    else if(info.status == REMOVED_MATCHING)
        cout << "Publisher  in topic " << m_attributes.topic.getTopicName() << " REMOVES Sub: " << info.remoteEndpointGuid << "*****************************"<<endl;
}


