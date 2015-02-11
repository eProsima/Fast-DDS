/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPListener.h
 *
 */

#ifndef WLPLISTENER_H_
#define WLPLISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
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
struct CacheChange_t;

/**
 * Class WLPListener that receives the liveliness messages asserting the liveliness of remote endpoints.
 * @ingroup LIVELINESS_MODULE
 */
class WLPListener: public ReaderListener {
public:
	/**
	 * Constructor
	 * @param Pointer to the WLP object.
	 */
	WLPListener(WLP* pwlp);
	virtual ~WLPListener();

	/**
	*
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const  change);
	/**
	* Separate the Key between the GuidPrefix_t and the liveliness Kind
	* @param key InstanceHandle_t to separate.
	* @param guidP GuidPrefix_t pointer to store the info.
	* @param liveliness Liveliness Kind Pointer.
	* @return True if correctly separated.
	*/
	bool separateKey(InstanceHandle_t& key,
			GuidPrefix_t* guidP,
			LivelinessQosPolicyKind* liveliness);
			
	/**
	* Compute the key from a CacheChange_t 
	* @param change
	*/
	bool computeKey(CacheChange_t* change);
	
	//!Auxiliary message.
	CDRMessage_t aux_msg;

private:
	WLP* mp_WLP;

};

} /* namespace rtps */
} /* namespace eprosima */
}
#endif
#endif /* WLPLISTENER_H_ */
