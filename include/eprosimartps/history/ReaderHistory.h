/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.h
 *
 */

#ifndef READERHISTORY_H_
#define READERHISTORY_H_

#include "eprosimartps/history/History.h"

namespace eprosima {
namespace rtps {

class ReaderHistory: public History {
public:
	ReaderHistory(Endpoint* endp,uint32_t payload_max_size=5000);
	virtual ~ReaderHistory();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERHISTORY_H_ */
