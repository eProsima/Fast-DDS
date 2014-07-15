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
#include <QMutex>


class dds::Publisher;

class ShapePublisher: public PublisherListener {
public:
	ShapePublisher(Participant* par);
	virtual ~ShapePublisher();
	PublisherAttributes m_attributes;
	Publisher* mp_pub;
	Participant* mp_participant;

	bool initPublisher();
    void write();
    void onPublicationMatched(MatchingInfo info);

    Shape m_shape;
    Shape m_drawShape;
    QMutex m_mutex;
    bool isInitialized;


};


#endif /* SHAPEPUBLISHER_H_ */
