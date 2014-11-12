/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file NackSupressionDuration.h
 *
 */

#ifndef NACKSUPRESSIONDURATION_H_
#define NACKSUPRESSIONDURATION_H_

#include "eprosimartps/utils/TimedEvent.h"

namespace eprosima {
namespace rtps {

class StatefulWriter;
class ReaderProxy;

/**
 * NackSupressionDuration class, used to avoid too "recent" NACK messages.
 * @ingroup WRITERMODULE
 */
class NackSupressionDuration:public TimedEvent {
public:
	virtual ~NackSupressionDuration();
	NackSupressionDuration(ReaderProxy* p_RP,boost::posix_time::milliseconds interval);

	void event(const boost::system::error_code& ec);

	ReaderProxy* mp_RP;
};

}
} /* namespace eprosima */

#endif /* NACKSUPRESSIONDURATION_H_ */
