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
 * @brief The ShapeContentFilter class, represents a content filter.
 */
class ShapeContentFilter
{
public:
    ShapeContentFilter(): m_maxX(MAX_DRAW_AREA_X),m_minX(0),m_maxY(MAX_DRAW_AREA_Y),m_minY(0),m_useFilter(false)
    {

    }
   ~ShapeContentFilter(){}
    uint32_t m_maxX;
    uint32_t m_minX;
    uint32_t m_maxY;
    uint32_t m_minY;
    bool m_useFilter;
};


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
    Shape m_drawShape;
    QMutex m_mutex;
    ShapeContentFilter m_filter;
    bool passFilter(ShapeType* shape);
    std::vector<GUID_t> m_remoteWriters;
    ColorInstanceHandle m_instances;

};

#endif /* SHAPESUBSCRIBER_H_ */
