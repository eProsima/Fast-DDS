/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderListener.h
 *
 */

#ifndef READERLISTENER_H_
#define READERLISTENER_H_

#include "fastrtps/rtps/common/MatchingInfo.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSReader;
struct CacheChange_t;

class ReaderListener
{
public:
	ReaderListener(){};
	virtual ~ReaderListener(){};
	virtual void onReaderMatched(RTPSReader* reader,MatchingInfo info){};
	virtual void onNewCacheChangeAdded(RTPSReader* reader, CacheChange_t* change){};
};

}
}
}

#endif /* READERLISTENER_H_ */
