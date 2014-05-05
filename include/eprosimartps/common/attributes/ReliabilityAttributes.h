/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReliabilityParameters.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RELIABILITYPARAMETERS_H_
#define RELIABILITYPARAMETERS_H_

namespace eprosima {
namespace rtps {

/**
 * Enum ReliabilityKind_t, reliability kind for reader or writer.
 */
typedef enum ReliabilityKind_t:octet{
	BEST_EFFORT = 0x01,//!< BEST_EFFORT
	RELIABLE=0x02    //!< RELIABLE
}ReliabilityKind_t;


class ReliabilityAttributes {
public:
	ReliabilityAttributes():reliabilityKind(BEST_EFFORT){};
	virtual ~ReliabilityAttributes();
	ReliabilityKind_t reliabilityKind;
};

class PublisherReliability:public ReliabilityAttributes
{
public:
	//!Period to send HB.
	Duration_t heartbeatPeriod;
	//!Delay response to a negative ack from a reader.
	Duration_t nackResponseDelay;
	//!Allows the reader to deny a response to a nack that arrives too soon after the change is sent.
	Duration_t nackSupressionDuration;
	//!The writer sends data periodically to all ReaderLocators (only in BEST_EFFORT - STATELESS combination).
	Duration_t resendDataPeriod;

	//uint8_t hb_per_max_samples;
	PublisherReliability(){
		heartbeatPeriod.seconds = 3;
		nackResponseDelay.nanoseconds = 200*1000*1000;
	}
	~PublisherReliability(){};
};

class SubscriberReliability:public ReliabilityAttributes
{
public:
	//!Delay the response to a HB.
	Duration_t heartbeatResponseDelay;
	//!Ignore too son received HB.
	Duration_t heartbeatSupressionDuration;
	SubscriberReliability()
	{
		heartbeatResponseDelay.nanoseconds = 500*1000*1000;
	}
	~SubscriberReliability(){};
};


} /* namespace rtps */
} /* namespace eprosima */

#endif /* RELIABILITYPARAMETERS_H_ */
