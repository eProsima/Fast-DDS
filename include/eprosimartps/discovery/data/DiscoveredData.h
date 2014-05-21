/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredData.h
 *
 *  Created on: May 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef DISCOVEREDDATA_H_
#define DISCOVEREDDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/reader/WriterProxy.h"

namespace eprosima {
namespace rtps {

class DiscoveredWriterData;
class DiscoveredReaderData;
class DiscoveredTopicData;
class DiscoveredParticipantData;

class DiscoveredData
{
public:
	DiscoveredData(){};
	virtual ~DiscoveredData(){};
	static bool ParameterList2DiscoveredWriterData(ParameterList_t& param,DiscoveredWriterData* wdata);
	static bool ParameterList2DiscoveredReaderData(ParameterList_t& param,DiscoveredReaderData* wdata);
	static bool ParameterList2DiscoveredTopicData(ParameterList_t& param,DiscoveredTopicData* wdata);
	static bool DiscoveredWriterData2ParameterList(DiscoveredWriterData& wdata,ParameterList_t* param);
	static bool DiscoveredReaderData2ParameterList(DiscoveredReaderData& wdata,ParameterList_t* param);
	static bool DiscoveredTopicData2ParameterList(DiscoveredTopicData& wdata,ParameterList_t* param);

	//static bool ParameterList2DiscoveredParticipantData(ParameterList_t& param,DiscoveredParticipantData* wdata);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDDATA_H_ */
