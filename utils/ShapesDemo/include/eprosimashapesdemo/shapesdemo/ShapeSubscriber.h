/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapeSubscriber.h
 *
 */

#ifndef SHAPESUBSCRIBER_H_
#define SHAPESUBSCRIBER_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"

#include <QMutex>

class ShapeSubscriber: public SubscriberListener {
public:
    ShapeSubscriber(Participant* par);
	virtual ~ShapeSubscriber();
	SubscriberAttributes m_attributes;
    Subscriber* mp_sub;
	Participant* mp_participant;


	bool initSubscriber();

	void onNewDataMessage();
	void onSubscriptionMatched();
	bool hasReceived;
    Shape m_shape;
    Shape m_drawShape;
    QMutex m_mutex;


};

#endif /* SHAPESUBSCRIBER_H_ */
