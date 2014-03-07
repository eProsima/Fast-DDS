/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>

#include "rtps_all.h"
#include "ParameterList_t.h"
#include "common/rtps_messages.h"

#ifndef PUBLISHER_H_
#define PUBLISHER_H_



namespace eprosima {

namespace rtps{
class RTPSWriter;
}

using namespace rtps;

/**
 * DDS namespace. Contains the public API to interact with the DDS-RTPS protocol.
 * @ingroup DDSMODULE
 */
namespace dds {


/**
 * Class Publisher, contains the public API to send new data. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
 */
class RTPS_DllAPI Publisher {
	friend class DomainParticipant;
public:
	Publisher();
	/**
	 * Create a publisher, assigning its pointer to the associated writer.
	 * Don't use directly, create Publisher using DomainParticipant static function.
	 */
	Publisher(RTPSWriter* Win);
	virtual ~Publisher();

	/**
	* Get the topic name.
	 */
	const std::string& getTopicName()
	{
		return topicName;
	}
	/**
	* Get the data type.
	 */
	const std::string& getTopicDataType()
	{
		return topicDataType;
	}

	/**
	 * Write data to the topic.
	 * @param Data Pointer to the data
	 * @return True if correct
	 * @par Calling example:
	 * @snippet dds_example.cpp ex_PublisherWrite
	 */
	bool write(void*Data);

	//Since there is no discovery:
	/** @name Discovery substitution methods.
	 * Since no discovery is implemented in this version, these methods are needed.
	 */
	/// @{
	/**
	 * Add a Reader Locator to the publisher.
	 * @param Loc Locator to add.
	 * @param expectsInlineQos Parameter to indicate whether or not the locator expects inlineQos with its Data messages.
	 * @return True if correct.
	 */
	bool addReaderLocator(Locator_t Loc,bool expectsInlineQos);


	///@}

	//! Parameter List associated with this Publisher (Qos).
	ParameterList_t ParamList;
private:
	RTPSWriter* W;
	//bool initialized;
	std::string topicName;
	std::string topicDataType;
	TypeReg_t type;


};

} /* namespace dds */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
