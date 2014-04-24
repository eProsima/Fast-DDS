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

/**
 * Class StatefulReader, specialization of RTPSReader than stores the state of the matched writers.
 * @ingroup READERMODULE
 */
class StatefulReader:public RTPSReader {
public:
	//StatefulReader();
	virtual ~StatefulReader();
	StatefulReader(const ReaderParams_t* param,uint32_t payload_size);
	/**
	 * Add a matched writer.
	 * @param[in] WP Pointer to the WriterProxy_t to add.
	 * @return True if correct.
	 */
	bool matched_writer_add(WriterProxy_t* WP);
	/**
	 * Remove a WriterProxy_t.
	 * @param[in] WP WriterProxy to remove.
	 * @return True if correct.
	 */
	bool matched_writer_remove(WriterProxy_t& WP);
	/**
	 * Remove a WriterProxy_t based on its GUID_t
	 * @param[in] writerGUID GUID_t of the writer to remove.
	 * @return True if correct.
	 */
	bool matched_writer_remove(GUID_t& writerGUID);
	/**
	 * Get a pointer to a WriterProxy_t.
	 * @param[in] writerGUID GUID_t of the writer to get.
	 * @param[out] WP Pointer to pointer of the WriterProxy.
	 * @return True if correct.
	 */
	bool matched_writer_lookup(GUID_t& writerGUID,WriterProxy** WP);
	//!Reliability parameters of the StatefulReader, times mainly.
	DDS_Reliability_t reliability;

	bool removeMinSeqCacheChange();
	bool removeAllCacheChange(int32_t* n_removed);

private:
	//! Vector containing pointers to the matched writers.
	std::vector<WriterProxy*> matched_writers;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULREADER_H_ */
