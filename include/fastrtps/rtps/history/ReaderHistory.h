/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderHistory.h
 *
 */

#ifndef READERHISTORY_H_
#define READERHISTORY_H_

#include "History.h"
#include "../common/CacheChange.h"
#include <map>
#include <set>


namespace boost
{
	namespace interprocess{ class interprocess_semaphore; }
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

//struct FirstLastSeqNum
//{
//	GUID_t wGuid;
//	SequenceNumber_t first;
//	SequenceNumber_t last;
//};

class WriterProxy;
class RTPSReader;

/**
 * Class ReaderHistory, container of the different CacheChanges of a reader
 * @ingroup READER_MODULE
 */
class ReaderHistory : public History {
	friend class RTPSReader;
public:
	/**
	* Constructor of the ReaderHistory. It needs a HistoryAttributes.
	*/
	RTPS_DllAPI ReaderHistory(const HistoryAttributes& att);
	RTPS_DllAPI virtual ~ReaderHistory();

	/**
	* Virtual method that is called when a new change is received.
	* In this implementation this method just calls add_change. The suer can overload this method in case
	* he needs to perform additional checks before adding the change.
	* @param change Pointer to the change
	* @return True if added.
	*/
	RTPS_DllAPI virtual bool received_change(CacheChange_t* change);

	/**
	 * Add a CacheChange_t to the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to add.
	 * @return True if added.
	 */
	RTPS_DllAPI bool add_change(CacheChange_t* a_change);

	/**
	 * Remove a CacheChange_t from the ReaderHistory.
	 * @param a_change Pointer to the CacheChange to remove.
	 * @return True if removed.
	 */
	RTPS_DllAPI bool remove_change(CacheChange_t* a_change);

	/**
	 * Sort the CacheChange_t from the History.
	 */
	RTPS_DllAPI void sortCacheChanges();
	/**
	 * Update the maximum and minimum sequenceNumber cacheChanges.
	 */
	RTPS_DllAPI void updateMaxMinSeqNum();

	//!Post to the semaphore
	RTPS_DllAPI void postSemaphore();
	//!Wait for the semaphore
	RTPS_DllAPI void waitSemaphore();

	RTPS_DllAPI bool thereIsRecordOf(GUID_t& guid, SequenceNumber_t& seq);

protected:
	//!Pointer to the reader
	RTPSReader* mp_reader;
	//!Pointer to the semaphore, used to halt execution until new message arrives.
	boost::interprocess::interprocess_semaphore* mp_semaphore;
	//!Information about changes already in History
	std::map<GUID_t, std::set<SequenceNumber_t>> m_historyRecord;//TODO sustituir por una clase que sea más efectiva,
																//que no guarde todos los numeros de secuencia rebidiso sino los que falten

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERHISTORY_H_ */
