/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include <iostream>
#include <sstream>
ShapesDemo::ShapesDemo():
    mp_participant(NULL),
    m_isInitialized(false)
{

}

ShapesDemo::~ShapesDemo()
{
	stop();
}

void ShapesDemo::init(uint32_t domainId)
{
    if(!m_isInitialized)
    {
        ParticipantAttributes pparam;
        pparam.name = "eProsimaParticipant";
        pparam.discovery.domainId = domainId;
        pparam.discovery.leaseDuration.seconds = 100;
        pparam.discovery.resendDiscoveryParticipantDataPeriod.seconds = 50;
        pparam.defaultSendPort = 10042;
        mp_participant = DomainParticipant::createParticipant(pparam);
        if(mp_participant!=NULL)
        {
            m_isInitialized = true;
        }
    }
}

void ShapesDemo::stop()
{
	DomainParticipant::stopAll();
	mp_participant = NULL;
	m_publishers.clear();
	m_subscribers.clear();
}
