/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPListener.h
 *
 */

#ifndef WLPLISTENER_H_
#define WLPLISTENER_H_

#include "fastrtps/rtps/reader/ReaderListener.h"

#include "fastrtps/rtps/common/Guid.h"
#include "fastrtps/rtps/common/InstanceHandle.h"
#include "fastrtps/qos/QosPolicies.h"
#include "fastrtps/qos/ParameterList.h"


using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{



namespace rtps {

class WLP;
class RTPSReader;
class CacheChange_t;

/**
 * Class WLPListener that receives the liveliness messages asserting the liveliness of remote endpoints.
 * @ingroup LIVELINESSMODULE
 */
class WLPListener: public ReaderListener {
public:
	WLPListener(WLP* pwlp);
	virtual ~WLPListener();

	void onNewCacheChangeAdded(RTPSReader* reader,CacheChange_t* change);
//	bool processParameterList(ParameterList_t* param,
//			GuidPrefix_t* guidP,
//			LivelinessQosPolicyKind* liveliness);

	bool separateKey(InstanceHandle_t& key,
			GuidPrefix_t* guidP,
			LivelinessQosPolicyKind* liveliness);
	bool computeKey(CacheChange_t* change);
	CDRMessage_t aux_msg;

private:
	WLP* mp_WLP;

};

} /* namespace rtps */
} /* namespace eprosima */
}
#endif /* WLPLISTENER_H_ */
