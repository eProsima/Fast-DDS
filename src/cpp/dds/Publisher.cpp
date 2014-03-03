/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/Publisher.h"
#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/ReaderLocator.h"
#include "eprosimartps/StatelessWriter.h"

namespace eprosima {
namespace dds {

Publisher::Publisher(){

}

Publisher::Publisher(RTPSWriter* Win) {
	// TODO Auto-generated constructor stub
	W = Win;
}

Publisher::~Publisher() {
	// TODO Auto-generated destructor stub
}

bool Publisher::write(void* Data) {
	//Convert data to serialized Payload
	cout << "Trying to write new data" << endl;
	SerializedPayload_t Payload;
	type.serialize(&Payload,Data);
	cout << "Serialization OK" << endl;
	//create new change
	CacheChange_t change;
	if(!W->new_change(ALIVE,&Payload,Data,&change))
	{
		cout << "falla new change" << endl;
		return false;
	}
	cout << "New change creado con seqnum: " << change.sequenceNumber.to64long() << endl;
	if(!W->writer_cache.add_change(change))
	{
		cout << "falla add change" << endl;
		return false;
	}
	return true;
}

bool Publisher::addReaderLocator(Locator_t Loc,bool expectsInlineQos)
{
	ReaderLocator RL;
	RL.expectsInlineQos = expectsInlineQos;
	RL.locator = Loc;
	if(W->stateType==STATELESS)
		((StatelessWriter*)W)->reader_locator_add(RL);
	//TODOG add proxy
	return true;
}


} /* namespace dds */
} /* namespace eprosima */


