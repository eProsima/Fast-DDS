/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapePublisher.cpp
 *
 */

#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"


ShapePublisher::ShapePublisher(Participant* par):
    mp_pub(NULL),
    mp_participant(par)
{
	// TODO Auto-generated constructor stub

}

ShapePublisher::~ShapePublisher()
{
	// TODO Auto-generated destructor stub
}

bool ShapePublisher::initPublisher()
{
    mp_pub = DomainParticipant::createPublisher(mp_participant,m_attributes);
    if(mp_pub !=NULL)
        return true;
    return false;
}

void ShapePublisher::write()
{
    if(mp_pub !=NULL)
    {
        mp_pub->write((void*)&this->m_shape.m_mainShape);
    }
}
