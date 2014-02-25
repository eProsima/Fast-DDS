/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/StatelessWriter.h"

namespace eprosima {
namespace rtps {

StatelessWriter::StatelessWriter() {
	// TODO Auto-generated constructor stub

}

StatelessWriter::StatelessWriter(WriterParams param) {
	// TODO Auto-generated constructor stub
	pushMode = param.pushMode;
	heartbeatPeriod = param.heartbeatPeriod;
	nackResponseDelay = param.nackResponseDelay;
	nackSupressionDuration = param.nackSupressionDuration;
	resendDataPeriod = param.resendDataPeriod;
	writer_cache.changes.reserve(param.HistorySize);
	lastChangeSequenceNumber = 0;
}


StatelessWriter::~StatelessWriter() {
	// TODO Auto-generated destructor stub
}

void StatelessWriter::reader_locator_add(ReaderLocator a_locator) {
	std::vector<CacheChange_t>::iterator it;
	for(it = writer_cache.changes.begin();it!=writer_cache.changes.end();it++){
		a_locator.unsent_changes.push_back(&(*it));
	}
	reader_locator.push_back(a_locator);
}

void StatelessWriter::reader_locator_remove(Locator_t locator) {
	std::vector<ReaderLocator>::iterator it;
	for(it=reader_locator.begin();it!=reader_locator.end();it++){
		if(it->locator == locator)
			reader_locator.erase(it);
	}
}

void StatelessWriter::unsent_changes_reset() {
	std::vector<ReaderLocator>::iterator rit;
	std::vector<CacheChange_t>::iterator cit;
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++){
		rit->unsent_changes.clear();
		for(cit=writer_cache.changes.begin();cit!=writer_cache.changes.end();cit++){
			rit->unsent_changes.push_back(&(*cit));
		}
	}
}

} /* namespace rtps */
} /* namespace eprosima */
