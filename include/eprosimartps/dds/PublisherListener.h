/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PublisherListener.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PUBLISHERLISTENER_H_
#define PUBLISHERLISTENER_H_

namespace eprosima {
namespace dds {

/**
 * Class PublisherListener, it should be used by the end user to implement specific callbacks to certain actions.
 * @ingroup DDSMODULE
 */
class PublisherListener {
public:
	PublisherListener();
	virtual ~PublisherListener();

	virtual void onPublicationMatched();

	virtual void onHistoryFull();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PUBLISHERLISTENER_H_ */
