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

#ifndef RTPS_ALL_H_
#define RTPS_ALL_H_

//USE THIS HEADER WITH CAUTION.
//IT INCLUDES ALL NECESSARY ELEMENTS TO COMPILE A TESTS USING THE PUBLIC API.

#include "fastrtps/common/types/all_common.h"

#include "fastrtps/pubsub/attributes/all_attributes.h"
#include "fastrtps/RTPSParticipant.h"
#include "fastrtps/pubsub/TopicDataType.h"
#include "fastrtps/pubsub/RTPSDomain.h"
#include "fastrtps/pubsub/Publisher.h"
#include "fastrtps/pubsub/Subscriber.h"
#include "fastrtps/pubsub/PublisherListener.h"
#include "fastrtps/pubsub/SubscriberListener.h"
#include "fastrtps/pubsub/RTPSParticipantListener.h"

#include "fastrtps/pubsub/SampleInfo.h"

#include "fastrtps/utils/IPFinder.h"
#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/qos/QosPolicies.h"

using namespace eprosima;
using namespace pubsub;
using namespace rtps;

#endif /* RTPS_ALL_H_ */
