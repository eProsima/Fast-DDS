/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWithRegistrationWriter.cpp
 *
 */

#include "RTPSWithRegistrationWriter.hpp"

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <gtest/gtest.h>

RTPSWithRegistrationWriter::RTPSWithRegistrationWriter(): listener_(*this), participant_(nullptr),
    writer_(nullptr), history_(nullptr), initialized_(false), matched_(0)
{
}

RTPSWithRegistrationWriter::~RTPSWithRegistrationWriter()
{
    if(participant_ != nullptr)
        RTPSDomain::removeRTPSParticipant(participant_);
    if(history_ != nullptr)
	delete(history_);
}

void RTPSWithRegistrationWriter::init(bool async)
{
	//Create participant
	RTPSParticipantAttributes pattr;
	pattr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	pattr.builtin.use_WriterLivelinessProtocol = true;
    pattr.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	participant_ = RTPSDomain::createParticipant(pattr);
    ASSERT_NE(participant_, nullptr);

	//Create writerhistory
	HistoryAttributes hattr;
	hattr.payloadMaxSize = 255;
	history_ = new WriterHistory(hattr);

	//Create writer
	WriterAttributes wattr;
    eprosima::fastrtps::WriterQos Wqos;
    configWriter(wattr, Wqos);

    // Asynchronous
    if(async)
        wattr.mode = ASYNCHRONOUS_WRITER;

	writer_ = RTPSDomain::createRTPSWriter(participant_, wattr, history_, &listener_);
    ASSERT_NE(writer_, nullptr);

    eprosima::fastrtps::TopicAttributes tattr;
	tattr.topicKind = NO_KEY;
	tattr.topicDataType = "string";
    configTopic(tattr);
	ASSERT_EQ(participant_->registerWriter(writer_, tattr, Wqos), true);

    initialized_ = true;
}

void RTPSWithRegistrationWriter::send(const std::list<uint16_t> &msgs)
{
    waitDiscovery();

	for(std::list<uint16_t>::const_iterator it = msgs.begin(); it != msgs.end(); ++it)
	{
		CacheChange_t * ch = writer_->new_change(ALIVE);

#if defined(_WIN32)
        ch->serializedPayload.length =
            (uint16_t)sprintf_s((char*)ch->serializedPayload.data, 255, "My example string %hu", *it) + 1;
#else
		ch->serializedPayload.length =
			sprintf((char*)ch->serializedPayload.data,"My example string %hu", *it) + 1;
#endif

		history_->add_change(ch);
	}
}

void RTPSWithRegistrationWriter::matched()
{
    std::unique_lock<std::mutex> lock(mutex_);
    ++matched_;
    cv_.notify_one();
}

void RTPSWithRegistrationWriter::waitDiscovery()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(matched_ == 0)
        cv_.wait_for(lock, std::chrono::seconds(10));

    ASSERT_NE(matched_, 0u);
}
