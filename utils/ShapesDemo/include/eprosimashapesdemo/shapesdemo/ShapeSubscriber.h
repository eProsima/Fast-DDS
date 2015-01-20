/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapeSubscriber.h
 *
 */

#ifndef SHAPESUBSCRIBER_H_
#define SHAPESUBSCRIBER_H_


#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapeHistory.h"
#include <QMutex>

#include "fastrtps/attributes/SubscriberAttributes.h"
#include "fastrtps/subscriber/SubscriberListener.h"

#include "fastrtps/fastrtps_fwd.h"

using namespace eprosima::fastrtps;

class ContentFilterSelector;

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

    void onNewDataMessage(Subscriber* sub);
    void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
    void adjustContentFilter(ShapeFilter& m_filter);
    void assignContentFilterPointer(ContentFilterSelector* p){mp_contentFilter = p;}
	bool hasReceived;

    QMutex m_mutex;

    std::vector<GUID_t> m_remoteWriters;
    ShapeHistory m_shapeHistory;
    TYPESHAPE m_shapeType;
    ContentFilterSelector* mp_contentFilter;

};

#endif /* SHAPESUBSCRIBER_H_ */
