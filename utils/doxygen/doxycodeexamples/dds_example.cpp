/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of eProsimaRTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * dds_example.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com
 */

//! [ex_readAllUnreadCache]
std::vector<(void*)(TypeStructure*)> vec; //TypeStructure is your own define structure for the topic
readAllUnreadCache(&vec);
for (elements in vec)
	TypeStructure tp = *(TypeStructure*)vec[i];
	//Do something with tp.
//! [ex_readAllUnreadCache]

//! [ex_readMinSeqUnread]
TypeStructure tp; //TypeStructure is your own define structure for the topic
readMinSeqUnread((void*)&tp);
//! [ex_readMinSeqUnread]


//! [ex_PublisherWrite]
TypeStructure tp; //TypeStructure is your own define structure for the topic
//Fill tp with the data you want.
write((void*)&tp);
//! [ex_PublisherWrite]


