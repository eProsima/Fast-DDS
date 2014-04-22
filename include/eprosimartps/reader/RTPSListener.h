/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DDSListener.h
 *
 *  Created on: Apr 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RTPSLISTENER_H_
#define RTPSLISTENER_H_

namespace eprosima {
namespace rtps {

/**
 * Class RTPSListener, to be used as a base by the user to program the actions to be performed when a new message is received.
 * @ingroup READERMODULE
 */
class RTPSListener {
public:
	RTPSListener() ;
	virtual ~RTPSListener();
	/**
	 * Virtual function to be implemented by the user containing the actions to be performed when a new Message is received.
	 */
	virtual void newMessageCallback();
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DDSLISTENER_H_ */
