/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file rtps_all.h
 *
 */

#ifndef FASTRTPS_ALL_H_
#define FASTRTPS_ALL_H_

//USER THIS HEADER TO CREATE RAPID PROTOTYPES AND TESTS
//DO NOT INCLUDE IN PROJETCTS WERE COMPILATION TIME OR SIZE IS REVELANT
//SINCE IT INCLUDES ALL NECESSARY HEADERS.

#include "rtps/common/all_common.h"

#include "Domain.h"

#include "participant/Participant.h"
#include "participant/ParticipantListener.h"
#include "publisher/Publisher.h"
#include "subscriber/Subscriber.h"
#include "publisher/PublisherListener.h"
#include "subscriber/SubscriberListener.h"


#include "attributes/ParticipantAttributes.h"
#include "attributes/PublisherAttributes.h"
#include "attributes/SubscriberAttributes.h"

#include "subscriber/SampleInfo.h"
#include "TopicDataType.h"

#include "utils/IPFinder.h"
#include "utils/RTPSLog.h"
#include "utils/eClock.h"
#include "utils/TimeConversion.h"

#include "qos/ParameterList.h"
#include "qos/QosPolicies.h"

#include "utils/RTPSLog.h"


#endif /* FASTRTPS_ALL_H_ */
