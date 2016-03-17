/*************************************************************************
 * Copyright (c) 20134 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderLocator.h
*/



#ifndef READERLOCATOR_H_
#define READERLOCATOR_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <vector>
#include "../common/Locator.h"
#include "../common/Guid.h"
#include "../common/SequenceNumber.h"
#include "../messages/RTPSMessageGroup.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CacheChange_t;

/**
 * Class ReaderLocator, contains information about a locator, without saving its state.
  * @ingroup WRITER_MODULE
 */
class ReaderLocator {
public:
	ReaderLocator();
	/**
	 * Constructor.
	* @param locator Locator to create the ReaderLocator.
	* @param expectsInlineQos Indicate that the ReaderLocator expects inline QOS.
	*/
	ReaderLocator(Locator_t& locator, bool expectsInlineQos);
	virtual ~ReaderLocator();
	//!Address of this ReaderLocator.
	Locator_t locator;
	//!Whether the Reader expects inlineQos with its data messages.
	bool expectsInlineQos;
	//!Vector containing pointers to the requested changes by this reader.
	std::vector<CacheChange_t*> requested_changes;
	//!Vector containing pointers to the unsent changes to this reader.
	std::vector<CacheChangeForGroup_t> unsent_changes;
	//!Number of times this locator has been used (in case different readers use the same locator).
	uint32_t n_used;

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
	bool next_unsent_change(const CacheChange_t** cpoin);
	/**
	 * Remove change from unsent list.
	 * @param cpoin Pointer to pointer
	 * @return True if correct.
	 */
	bool remove_unsent_change(CacheChange_t* cpoin);

};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* READERLOCATOR_H_ */
