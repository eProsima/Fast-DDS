/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatelessReader.h
 *  StatelessReader class.
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/reader/RTPSReader.h"

#include <boost/bind.hpp>

#ifndef STATELESSREADER_H_
#define STATELESSREADER_H_

namespace eprosima {
namespace rtps {

/**
 * Class StatelessReader,
 * @ingroup READERMODULE
 */
class StatelessReader: public RTPSReader {
public:
	StatelessReader();
	virtual ~StatelessReader();
	StatelessReader(ReaderParams_t* param);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* STATELESSREADER_H_ */
