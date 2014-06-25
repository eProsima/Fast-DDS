/*************************************************************************
 * Copyright (c) 20134 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderLocator.h
*/



#ifndef READERLOCATOR_H_
#define READERLOCATOR_H_
#include <vector>
#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/common/types/SequenceNumber.h"

namespace eprosima {
namespace rtps {

class HistoryCache;
struct CacheChange_t;

/**
 * Class ReaderLocator, contains information about a locator, without saving its state.
  * @ingroup WRITERMODULE
 */
class ReaderLocator {
public:
	ReaderLocator();
	ReaderLocator(Locator_t& locator, bool expectsInlineQos);
	virtual ~ReaderLocator();
	//!Address of this ReaderLocator.
	Locator_t locator;
	//!Whether the Reader expects inlineQos with its data messages.
	bool expectsInlineQos;
	//!Vector containing pointers to the requested changes by this reader.
	std::vector<CacheChange_t*> requested_changes;
	//!Vector containing pointers to the unsent changes to this reader.
	std::vector<CacheChange_t*> unsent_changes;
	/**
	 * Retrieve next requested change from the HistoryCache.
	 * @param cpoin Pointer to pointer.
	 * @return True if correct.
	 */
	bool next_requested_change(CacheChange_t** cpoin);
	/**
	 * Remove change from requested list.
	 * @param cpoin Pointer to change.
	 * @return True if correct
	 */
	bool remove_requested_change(CacheChange_t* cpoin);
	/**
	 * Get next unsent change from the History.
	 * @param cpoin Pointer to pointer of CacheChange_t
	 * @return True if correct.
	 */
	bool next_unsent_change(CacheChange_t** cpoin);
	/**
	 * Remove change from unsent list.
	 * @param cpoin Pointer to pointer
	 * @return True if correct.
	 */
	bool remove_unsent_change(CacheChange_t* cpoin);
	/**
	 * Change the requested changes to the provided set.
	 * @param seqs Vector of SequenceNumbers.
	 * @param readerGUI
	 * @param history
	 */
//	void requested_changes_set(std::vector<SequenceNumber_t>& seqs,GUID_t& readerGUI,HistoryCache* history);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERLOCATOR_H_ */
