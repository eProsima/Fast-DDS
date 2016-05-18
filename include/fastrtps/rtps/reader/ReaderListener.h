/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderListener.h
 *
 */

#ifndef READERLISTENER_H_
#define READERLISTENER_H_

#include "../common/MatchingInfo.h"
#include <mutex>
namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSReader;
struct CacheChange_t;

/**
* Class ReaderListener, to be used by the user to override some of is virtual method to program actions to certain events.
*  @ingroup READER_MODULE
*/
class RTPS_DllAPI ReaderListener
{
public:
	ReaderListener(){};
	virtual ~ReaderListener(){};
	
	/**
	* This method is invoked when a new reader matches
	* @param reader Matching reader
	* @param info Matching information of the reader
	*/
	virtual void onReaderMatched(RTPSReader* reader, MatchingInfo& info){(void)reader; (void)info;};
	
	/**
	* This method is called when a new CacheChange_t is added to the ReaderHistory.
	* @param reader Pointer to the reader.
	* @param change Pointer to the CacheChange_t. This is a const pointer to const data
	* to indicate that the user should not dispose of this data himself.
	* To remove the data call the remove_change method of the ReaderHistory.
	* reader->getHistory()->remove_change((CacheChange_t*)change).
	*/
	virtual void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change){(void)reader; (void)change;};
};

class RTPS_DllAPI InfectableReaderListener: public ReaderListener
{
public:
	InfectableReaderListener():attached_listener(nullptr){};
	virtual ~InfectableReaderListener(){};

	virtual void onReaderMatched(RTPSReader* /*reader*/, MatchingInfo& /*info*/){};
	virtual void onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const /*change*/){};

	void attachListener(ReaderListener *secondary_listener);
        void detachListener();	
	bool hasReaderAttached();
	ReaderListener* getAttachedListener();	
protected:
	std::mutex attached_listener_mutex;
	ReaderListener* attached_listener;

};

//Namespace enders
}
}
}

#endif /* READERLISTENER_H_ */
