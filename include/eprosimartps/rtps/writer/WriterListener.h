/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterListener.h
 *
 */

#ifndef WRITERLISTENER_H_
#define WRITERLISTENER_H_

#include "eprosimartps/rtps/common/MatchingInfo.h"

namespace eprosima
{
namespace rtps{


class WriterListener
{
public:
	WriterListener(){};
	virtual ~WriterListener(){};
	virtual void onWriterMatched(MatchingInfo info){};
};

}
}



#endif /* WRITERLISTENER_H_ */
