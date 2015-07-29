/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReceivedChanges.h
 *
 */

#ifndef RECEIVEDCHANGES_H_
#define RECEIVEDCHANGES_H_

#include "fastrtps/rtps/common/SequenceNumber.h"

namespace eprosima {
namespace fastrtps {
namespace rtps{

class ReceivedChanges {
public:
	ReceivedChanges();
	virtual ~ReceivedChanges();

	bool add(SequenceNumber_t& s);
private:
	bool insertMissingSequenceNumbers(SequenceNumber_t& s);
	bool missingSequenceNumberReceived(SequenceNumber_t& s);

	std::vector<SequenceNumber_t> m_missingChanges;
	SequenceNumber_t m_largestSequenceNumberReceived;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RECEIVEDCHANGES_H_ */
