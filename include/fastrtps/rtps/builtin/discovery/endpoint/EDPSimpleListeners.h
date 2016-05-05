/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListeners.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../../../reader/ReaderListener.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class EDPSimple;
class RTPSReader;
struct CacheChange_t;

/**
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimplePUBListener:public InfectableReaderListener{
public:
	/**
	Constructor
	* @param p Pointer to the EDPSimple associated with this listener.
	*/
	EDPSimplePUBListener(EDPSimple* p):mp_SEDP(p){};
	virtual ~EDPSimplePUBListener(){};
	/**
	* Virtual method, 
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const  change);
	/**
	* Compute the Key from a CacheChange_t
	* @param change Pointer to the change.
	*/
	bool computeKey(CacheChange_t* change);
	//!Pointer to the EDPSimple
	EDPSimple* mp_SEDP;
};
/**
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 *@ingroup DISCOVERY_MODULE
 */
class EDPSimpleSUBListener:public InfectableReaderListener{
public:
	/**
	* @param p
	*/
	EDPSimpleSUBListener(EDPSimple* p):mp_SEDP(p){}

	virtual ~EDPSimpleSUBListener(){}
	/**
	* @param reader
	* @param change
	*/
	void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);
	/**
	* @param change
	*/
	bool computeKey(CacheChange_t* change);
	//!Pointer to the EDPSimple
	EDPSimple* mp_SEDP;
};

} /* namespace rtps */
}
} /* namespace eprosima */
#endif
#endif /* EDPSIMPLELISTENER_H_ */
