/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessReader.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "rtps_all.h"
#include "RTPSReader.h"

#include <boost/bind.hpp>

#ifndef STATELESSREADER_H_
#define STATELESSREADER_H_

namespace eprosima {
namespace rtps {

class StatelessReader: public RTPSReader {
public:
	StatelessReader();
	virtual ~StatelessReader();
	void init(ReaderParams_t);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSREADER_H_ */
