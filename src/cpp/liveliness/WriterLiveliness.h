/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterLiveliness.h
 *
 */

#ifndef WRITERLIVELINESS_H_
#define WRITERLIVELINESS_H_

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class StatefulWriter;
class StatefulReader;

class WriterLiveliness {
public:
	WriterLiveliness(ParticipantImpl* p);
	virtual ~WriterLiveliness();

	StatefulWriter* mp_builtinParticipantMessageWriter;
	StatefulReader* mp_builtinParticipantMessageReader;

	bool createEndpoints();

private:

	ParticipantImpl* mp_participant;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERLIVELINESS_H_ */
