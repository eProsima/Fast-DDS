// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/WriterQos.h>

#include <string>
#include <list>
#include <stdio.h>

class my_WriterListener: public eprosima::fastrtps::rtps::WriterListener
{
    public:
        my_WriterListener();
        ~my_WriterListener() override;
        void onWriterMatched(
                eprosima::fastrtps::rtps::RTPSWriter* writer,
                eprosima::fastrtps::rtps::MatchingInfo& info) override;
        int n_matched;
};


class UserDefinedTransportExampleWriter
{
    private:

    my_WriterListener *my_listener;
    void waitformatching();

    public:

        UserDefinedTransportExampleWriter();
        ~UserDefinedTransportExampleWriter();

        void init();
        bool isInitialized();
    void sendData();

    private:
        eprosima::fastrtps::rtps::RTPSParticipantAttributes pattr;
        eprosima::fastrtps::rtps::RTPSParticipant *my_participant;
        eprosima::fastrtps::rtps::WriterAttributes wattr;
        eprosima::fastrtps::rtps::RTPSWriter *my_writer;
        eprosima::fastrtps::rtps::HistoryAttributes hattr;
        eprosima::fastrtps::rtps::WriterHistory *my_history;
        eprosima::fastrtps::TopicAttributes tattr;
        eprosima::fastrtps::WriterQos wqos;
        bool initialized_;
};


