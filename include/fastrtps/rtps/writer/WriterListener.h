/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterListener.h
 *
 */

#ifndef WRITERLISTENER_H_
#define WRITERLISTENER_H_

#include "fastrtps/rtps/common/MatchingInfo.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSWriter;

class WriterListener
{
public:
	WriterListener(){};
	virtual ~WriterListener(){};
	virtual void onWriterMatched(RTPSWriter* writer,MatchingInfo info){};
};
}
}
}



#endif /* WRITERLISTENER_H_ */
