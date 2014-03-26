/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef STATEFULREADER_H_
#define STATEFULREADER_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/WriterProxy.h"
#include <boost/bind.hpp>

namespace eprosima {
namespace rtps {

class StatefulReader:public RTPSReader {
public:
	//StatefulReader();
	virtual ~StatefulReader();
	StatefulReader(ReaderParams_t* param,uint32_t payload_size);

	bool matched_writer_add(WriterProxy_t* WP);
	bool matched_writer_remove(WriterProxy_t WP);
	bool matched_writer_remove(GUID_t writerGUID);
	bool matched_writer_lookup(GUID_t writerGUID,WriterProxy** WP);

	DDS_Reliability_t reliability;
private:
	std::vector<WriterProxy*> matched_writers;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULREADER_H_ */
