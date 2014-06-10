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
*/

#ifndef DISCOVEREDDATA_H_
#define DISCOVEREDDATA_H_

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/discovery/data/DiscoveredWriterData.h"
#include "eprosimartps/discovery/data/DiscoveredReaderData.h"


namespace eprosima {
namespace rtps {

/**
 * Class DiscoveredData, contains static methods to convert between ParameterLists and DiscoveredData classes.
 */
class DiscoveredData
{
public:
	DiscoveredData(){};
	virtual ~DiscoveredData(){};
	/**
	 * Convert ParameterList to an DiscoveredWriterData object.
	 * @param param Reference to the ParameterList_t.
	 * @param wdata Pointer to the DWD data object.
	 * @return True if correct.
	 */
	static bool ParameterList2DiscoveredWriterData(ParameterList_t& param,DiscoveredWriterData* wdata);
	/**
	 * Convert ParameterList to an DiscoveredReaderData object.
	 * @param param Reference to the ParameterList_t.
	 * @param rdata Pointer to the DRD data object.
	 * @return True if correct.
	 */
	static bool ParameterList2DiscoveredReaderData(ParameterList_t& param,DiscoveredReaderData* rdata);

	/**
	 * Convert DWD object to a ParameterList.
	 * @param wdata Reference to DWD object.
	 * @param param Pointer to parameterList.
	 * @return True if correct.
	 */
	static bool DiscoveredWriterData2ParameterList(DiscoveredWriterData& wdata,ParameterList_t* param);
	/**
	 * Convert DRD object to a ParameterList.
	 * @param rdata Reference to DRD object.
	 * @param param Pointer to parameterList.
	 * @return True if correct.
	 */
	static bool DiscoveredReaderData2ParameterList(DiscoveredReaderData& rdata,ParameterList_t* param);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDDATA_H_ */
