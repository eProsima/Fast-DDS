/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessWriter.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"
#include "RTPSWriter.h"
#include "ReaderLocator.h"
#include <boost/bind.hpp>

#ifndef STATELESSWRITER_H_
#define STATELESSWRITER_H_

namespace eprosima {
namespace rtps {

/**
 * Class StatelessWriter, specialization of RTPSWriter that manages writers that don't keep state of the matched readers.
  * @ingroup RTPSMODULE
 */
class StatelessWriter : public RTPSWriter
{
public:
	StatelessWriter();
	virtual ~StatelessWriter();
	void init(WriterParams_t);
	Duration_t resendDataPeriod;
	std::vector<ReaderLocator> reader_locator;
	bool reader_locator_add(ReaderLocator locator);
	bool reader_locator_remove(Locator_t locator);
	void unsent_changes_reset();
	//void unsent_change_add(SequenceNumber_t sn);
	void unsent_change_add(CacheChange_t* p);
	void unsent_changes_not_empty();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSWRITER_H_ */
