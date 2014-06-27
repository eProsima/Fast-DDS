/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"

ShapesDemo::ShapesDemo():
	mp_participant(NULL)
{

}

ShapesDemo::~ShapesDemo()
{
	stop();
}

void ShapesDemo::init()
{

}

void ShapesDemo::stop()
{
	DomainParticipant::stopAll();
	mp_participant = NULL;
	m_publishers.clear();
	m_subscribers.clear();
}
