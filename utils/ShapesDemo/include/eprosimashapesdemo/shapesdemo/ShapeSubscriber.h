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
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include <QMutex>




/**
 * @brief The ShapeSubscriber class, implements a Subscriber to receive shapes.
 */
class ShapeSubscriber: public SubscriberListener {
public:
    ShapeSubscriber(Participant* par);
	virtual ~ShapeSubscriber();
	SubscriberAttributes m_attributes;
    Subscriber* mp_sub;
	Participant* mp_participant;
    /**
     * @brief Initialize the subscriber
     * @return True if correct.
     */
	bool initSubscriber();

	void onNewDataMessage();
    void onSubscriptionMatched(MatchingInfo info);
    void adjustContentFilter(ShapeContentFilter& m_filter);
	bool hasReceived;
    Shape m_shape;
    QMutex m_mutex;
    ShapeContentFilter m_filter;
    bool passFilter(ShapeType* shape);
    std::vector<GUID_t> m_remoteWriters;
    ColorInstanceHandle m_instances;

};

#endif /* SHAPESUBSCRIBER_H_ */
