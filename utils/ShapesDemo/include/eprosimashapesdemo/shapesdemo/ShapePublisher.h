/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapePublisher.h
 *
 */

#ifndef SHAPEPUBLISHER_H_
#define SHAPEPUBLISHER_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"

using namespace eprosima;
using namespace dds;
using namespace rtps;

class dds::Publisher;

class ShapePublisher {
public:
	ShapePublisher(Participant* par);
	virtual ~ShapePublisher();
	PublisherAttributes m_attributes;
	Publisher* mp_pub;
	Participant* mp_participant;
	Shape m_shape;

	bool initPublisher();
    void write();
};


#endif /* SHAPEPUBLISHER_H_ */
