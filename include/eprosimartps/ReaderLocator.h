/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ReaderLocator.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "rtps_all.h"
#include "HistoryCache.h"

#ifndef READERLOCATOR_H_
#define READERLOCATOR_H_

namespace eprosima {
namespace rtps {


/**
 * Class ReaderLocator, contains information about a locator, without saving its state.
 */
class ReaderLocator {
public:
	ReaderLocator();
	ReaderLocator(Locator_t locator, bool expectsInlineQos);
	virtual ~ReaderLocator();
	Locator_t locator;
	bool expectsInlineQos;
	std::vector<CacheChange_t*> requested_changes;
	std::vector<CacheChange_t*> unsent_changes;
	//TODO Methods
	bool next_requested_change(CacheChange_t** cpoin);
	bool remove_requested_change(CacheChange_t* cpoin);
	bool next_unsent_change(CacheChange_t** cpoin);
	bool remove_unsent_change(CacheChange_t* cpoin);
	void requested_changes_set(std::vector<SequenceNumber_t> seqs,HistoryCache* history);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERLOCATOR_H_ */
