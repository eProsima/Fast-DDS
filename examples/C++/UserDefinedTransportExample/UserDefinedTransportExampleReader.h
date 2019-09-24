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
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <string>
#include <list>

class my_ReaderListener: public eprosima::fastrtps::rtps::ReaderListener
{
    public:
        int n_received;
        my_ReaderListener();
        ~my_ReaderListener() override;
        void onNewCacheChangeAdded(
                eprosima::fastrtps::rtps::RTPSReader* reader,
                const eprosima::fastrtps::rtps::CacheChange_t* const change) override;

        void onReaderMatched(
                eprosima::fastrtps::rtps::RTPSReader* reader,
                eprosima::fastrtps::rtps::MatchingInfo& info) override;
};

class UserDefinedTransportExampleReader
{
    private:
        my_ReaderListener *my_listener;
    public:
        UserDefinedTransportExampleReader();
        ~UserDefinedTransportExampleReader();
        void init();
        bool isInitialized();
        bool read();
    private:
        eprosima::fastrtps::rtps::RTPSParticipantAttributes pattr;
        eprosima::fastrtps::rtps::RTPSParticipant *my_participant;
        eprosima::fastrtps::rtps::ReaderAttributes rattr;
        eprosima::fastrtps::rtps::RTPSReader *my_reader;
        eprosima::fastrtps::rtps::HistoryAttributes hattr;
        eprosima::fastrtps::ReaderQos rqos;
        eprosima::fastrtps::TopicAttributes tattr;
        eprosima::fastrtps::rtps::ReaderHistory *my_history;
        bool initialized_;
};
