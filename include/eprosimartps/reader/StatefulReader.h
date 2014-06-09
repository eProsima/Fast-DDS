/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulReader.h
*/

#ifndef STATEFULREADER_H_
#define STATEFULREADER_H_


#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/reader/WriterProxy.h"


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
	StatefulReader(const SubscriberAttributes& wParam,
			const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype);
	/**
	 * Add a matched writer.
	 * @param[in] WP Pointer to the WriterProxy_t to add.
	 * @return True if correct.
	 */
	bool matched_writer_add(WriterProxy_t& WP);
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


	bool readNextCacheChange(void*data,SampleInfo_t* info);
	bool takeNextCacheChange(void*data,SampleInfo_t* info);
	bool isUnreadCacheChange();

	size_t getMatchedPublishers(){return matched_writers.size();}


private:
	SubscriberTimes m_SubTimes;
	//! Vector containing pointers to the matched writers.
	std::vector<WriterProxy*> matched_writers;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATEFULREADER_H_ */
