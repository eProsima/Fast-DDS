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

#include "eprosimartps/dds/attributes/all_attributes.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/dds/SubscriberListener.h"

#include "eprosimartps/dds/SampleInfo.h"

#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/DDSQosPolicies.h"

using namespace eprosima;
using namespace dds;
using namespace rtps;

#endif /* RTPS_ALL_H_ */
