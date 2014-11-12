/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
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

#include "eprosimartps/common/types/all_common.h"

#include "eprosimartps/pubsub/attributes/all_attributes.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/pubsub/RTPSDomain.h"
#include "eprosimartps/pubsub/Publisher.h"
#include "eprosimartps/pubsub/Subscriber.h"
#include "eprosimartps/pubsub/PublisherListener.h"
#include "eprosimartps/pubsub/SubscriberListener.h"
#include "eprosimartps/pubsub/ParticipantListener.h"

#include "eprosimartps/pubsub/SampleInfo.h"

#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/QosPolicies.h"

using namespace eprosima;
using namespace pubsub;
using namespace rtps;

#endif /* RTPS_ALL_H_ */
