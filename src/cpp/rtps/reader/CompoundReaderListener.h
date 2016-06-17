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

#ifndef COMPOUNDREADERLISTENER_H_
#define COMPOUNDREADERLISTENER_H_

#include <fastrtps/rtps/reader/ReaderListener.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Class CompoundReaderListener. To be used by the Built-in protocols to allow a user ReaderListener to provide additional callbacks
 * on events belonging to the built-in protocols. This is default option when using SimpleEDP.
 * @ingroup READER_MODULE
 */
class CompoundReaderListener: public ReaderListener
{
public:
	CompoundReaderListener():attached_listener(nullptr){};
	virtual ~CompoundReaderListener(){};

	virtual void onReaderMatched(RTPSReader* /*reader*/, MatchingInfo& /*info*/){};
	virtual void onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const /*change*/){};

	/**
	 * Attaches a secondary ReaderListener to this ReaderListener, so both callbacks are executed on a single event.
	 * @param secondary_listener to attach
	 */
	void attachListener(ReaderListener *secondary_listener);
	/**
	 * Detaches any currently used secondary ReaderListener, but does not manage its destruction
	 */
        void detachListener();	
	/**
	 * Checks if there is currently a secondary ReaderListener attached to this element
	 * @return True if there is a reader attached
	 */
	bool hasReaderAttached();
	/**
	 * Get a pointer to the secondary ReaderListener attached to this ReaderListener, in case there is one
	 * @return ReaderListener pointer to the secondary listener
	 */
	ReaderListener* getAttachedListener();	
protected:
	//! Mutex to ensure exclusive access to the attachedListener
	std::mutex attached_listener_mutex;
	//! Pointer to the secondary ReaderListener, should there be one attached
	ReaderListener* attached_listener;

};

//Namespace enders
}
}
}

#endif /* COMPOUNDREADERLISTENER_H_ */
