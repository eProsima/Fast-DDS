/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPListener.h
 *
 */

#ifndef WLPLISTENER_H_
#define WLPLISTENER_H_

#include "eprosimartps/pubsub/SubscriberListener.h"
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/qos/QosPolicies.h"
#include "eprosimartps/common/types/InstanceHandle.h"
#include "eprosimartps/qos/ParameterList.h"


using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

class WLP;

/**
 * Class WLPListener that receives the liveliness messages asserting the liveliness of remote endpoints.
 * @ingroup LIVELINESSMODULE
 */
class WLPListener: public SubscriberListener {
public:
	WLPListener(WLP* pwlp);
	virtual ~WLPListener();

	void onNewDataMessage();
	bool processParameterList(ParameterList_t* param,
			GuidPrefix_t* guidP,
			LivelinessQosPolicyKind* liveliness);

	bool separateKey(InstanceHandle_t& key,
			GuidPrefix_t* guidP,
			LivelinessQosPolicyKind* liveliness);

private:
	WLP* mp_WLP;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLPLISTENER_H_ */
