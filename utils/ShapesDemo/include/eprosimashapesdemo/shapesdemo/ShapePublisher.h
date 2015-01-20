/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ShapePublisher.h
 *
 */

#ifndef SHAPEPUBLISHER_H_
#define SHAPEPUBLISHER_H_

#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include <QMutex>
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/publisher/PublisherListener.h"

#include "fastrtps/fastrtps_fwd.h"

using namespace eprosima::fastrtps;

/**
 * @brief The ShapePublisher class, implements a Publisher to transmit shapes.
 */
class ShapePublisher: public PublisherListener {
public:
    ShapePublisher(Participant* par);
	virtual ~ShapePublisher();
	PublisherAttributes m_attributes;
	Publisher* mp_pub;
    Participant* mp_participant;
    /**
     * @brief Initialize the publisher.
     * @return  True if correct.
     */
	bool initPublisher();
    /**
     * @brief Write the shape.
     */
    void write();
    /**
     * @brief onPublicationMatched
     * @param info
     */
    void onPublicationMatched(Publisher* pub,MatchingInfo& info);

    Shape m_shape;
    QMutex m_mutex;
    bool isInitialized;
    bool hasWritten;


};


#endif /* SHAPEPUBLISHER_H_ */
