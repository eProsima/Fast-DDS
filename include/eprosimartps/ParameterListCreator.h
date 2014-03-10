/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParameterListCreator.h
 *	Methods to create a Parameter List and add parameters.
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "rtps_all.h"
#include "ParameterList_t.h"


#ifndef PARAMETERLISTCREATOR_H_
#define PARAMETERLISTCREATOR_H_

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {

/**
 * Class ParameterListCreator to add different parameters to a ParameterList
 * @ingroup DDSMODULE
 */
class RTPS_DllAPI ParameterListCreator {
public:
	ParameterListCreator();
	virtual ~ParameterListCreator();
	/**
	 * Update the CDRMessage_t of the parameter list.
	 * @param[in,out] pList Pointer to the ParameterList.
	 * @return True if correct.
	 */
	static bool updateMsg(ParameterList_t* pList);
	/*!
	 * Read a ParameterList from a CDRmessage.
	 * The message read position must be correct or the reading with return false.
	 * @param[out] plist Pointer to the list.
	 * @param[in] msg Pointer to the message.
	 * @param[out] size Pointer to indicate the final size of the parameter list.
	 */
	static bool readParamListfromCDRmessage(ParameterList_t* plist,CDRMessage_t* msg,uint32_t* size);

	/**@name Add Parameter_t to ParameterList_t methods
	 * @param[out] plist The list where to add the parameter.
	 * @param[in] pid The PID of the message.
	 * @param[in] data The type of data associated with this parameter.
	 * @return True if correct
	 */
	///@{
	static bool addParameterLocator(ParameterList_t* plist,ParameterId_t pid,rtps::Locator_t loc);
	static bool addParameterString(ParameterList_t* plist,ParameterId_t pid,std::string in_str);
	static bool addParameterPort(ParameterList_t* plist,ParameterId_t pid,uint32_t port);
	///@}
};





} /* namespace dds */
} /* namespace eprosima */

#endif /* PARAMETERLISTCREATOR_H_ */
