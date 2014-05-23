/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessWriter.h
 *   StatelessWriter class.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/common/types/Time_t.h"
#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/writer/ReaderLocator.h"
#include "eprosimartps/dds/attributes/PublisherAttributes.h"


#ifndef STATELESSWRITER_H_
#define STATELESSWRITER_H_

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
 * @ingroup WRITERMODULE
 */
class StatelessWriter : public RTPSWriter
{
public:
	//StatelessWriter();
	virtual ~StatelessWriter();
	StatelessWriter(const PublisherAttributes& wParam,
			const GuidPrefix_t&guidP, const EntityId_t& entId);


	/**
	 * Add a ReaderLocator to the Writer.
	 * @param locator ReaderLocator to add.
	 * @return True if correct.
	 */
	bool reader_locator_add(ReaderLocator& locator);
	bool reader_locator_add(Locator_t& locator,bool expectsInlineQos);
	/**
	 * Remove a ReaderLocator from this writer.
	 * @param locator Locator to remove.
	 * @return True if correct.
	 */
	bool reader_locator_remove(Locator_t& locator);
	/**
	 * Reset the unsent changes. All the changes currently in the HistoryCache are added to all teh ReaderLocator associated
	 * with this StatelessWriter, discarding the previous ones.
	 */
	void unsent_changes_reset();

	/**
	 * Add a specific change to all ReaderLocators.
	 * @param p Pointer to the change.
	 */
	void unsent_change_add(CacheChange_t* p);
	/**
	 * Method to indicate that there are changes not sent in some of all ReaderLocator.
	 */
	void unsent_changes_not_empty();

	 bool removeMinSeqCacheChange();
	 bool removeAllCacheChange(int32_t* n_removed);
private:
	 Duration_t resendDataPeriod; //FIXME: Not used yet.
	 std::vector<ReaderLocator> reader_locator;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSWRITER_H_ */
