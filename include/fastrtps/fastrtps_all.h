/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
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

#include "fastrtps/rtps/common/all_common.h"

#include "fastrtps/Domain.h"

#include "fastrtps/participant/Participant.h"
#include "fastrtps/participant/ParticipantListener.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"


#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/attributes/SubscriberAttributes.h"

#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/TopicDataType.h"

#include "fastrtps/utils/IPFinder.h"
#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/qos/QosPolicies.h"

#include "fastrtps/utils/RTPSLog.h"


#endif /* FASTRTPS_ALL_H_ */
