/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessReader.h
 *  StatelessReader class.
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#ifndef STATELESSREADER_H_
#define STATELESSREADER_H_


#include "eprosimartps/reader/RTPSReader.h"

#include "eprosimartps/dds/attributes/SubscriberAttributes.h"



namespace eprosima {
namespace rtps {

/**
 * Class StatelessReader, specialization of the RTPSReader.
 * @ingroup READERMODULE
 */
class StatelessReader: public RTPSReader {
public:
	virtual ~StatelessReader();
	StatelessReader(const SubscriberAttributes& wParam,
			const GuidPrefix_t&guidP, const EntityId_t& entId,DDSTopicDataType* ptype);



	 bool readNextCacheChange(void*data,SampleInfo_t* info);
	 bool takeNextCacheChange(void*data,SampleInfo_t* info);
	 bool isUnreadCacheChange();

	 size_t getMatchedPublishers(){return 0;}

	 bool matched_writer_add(const GUID_t& guid);
	 bool matched_writer_remove(const GUID_t& guid);

private:
	 //!List of GUID_t os matched writers.
	 //!Is only used in the Discovery, to correctly notify the user using SubscriptionListener::onSubscriptionMatched();
	 std::vector<GUID_t> m_matched_writers;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSREADER_H_ */
