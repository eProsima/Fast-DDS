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

/**
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include <asio.hpp>

#include "ThroughputTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/rtps/attributes/PropertyPolicy.h>
//#include <fastrtps/types/DynamicTypeBuilderFactory.h>
//#include <fastrtps/types/DynamicDataFactory.h>
//#include <fastrtps/types/DynamicTypeBuilder.h>
//#include <fastrtps/types/DynamicTypeBuilderPtr.h>
//#include <fastrtps/types/TypeDescriptor.h>
//#include <fastrtps/types/MemberDescriptor.h>
//#include <fastrtps/types/DynamicType.h>
//#include <fastrtps/types/DynamicData.h>
//#include <fastrtps/types/DynamicPubSubType.h>

#include <condition_variable>
#include <chrono>
#include <map>
#include <vector>
#include <string>

class ThroughputPublisher
{
    public:

        ThroughputPublisher(bool reliable, uint32_t pid, bool hostname, bool export_csv,
                const std::string& export_prefix,
                const eprosima::fastrtps::rtps::PropertyPolicy& part_property_policy,
                const eprosima::fastrtps::rtps::PropertyPolicy& property_policy,
                const std::string& sXMLConfigFile, bool dynamic_types, int forced_domain);
        virtual ~ThroughputPublisher();
        eprosima::fastrtps::Participant* mp_par;
        eprosima::fastrtps::Publisher* mp_datapub;
        eprosima::fastrtps::Publisher* mp_commandpub;
        eprosima::fastrtps::Subscriber* mp_commandsub;
        std::chrono::steady_clock::time_point t_start_, t_end_;
        std::chrono::duration<double, std::micro> t_overhead_;
        std::mutex mutex_;
        int disc_count_;
        std::condition_variable disc_cond_;
        std::mutex dataMutex_;
        int data_disc_count_;
        std::condition_variable data_disc_cond_;

        class DataPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                DataPubListener(ThroughputPublisher& up);
                virtual ~DataPubListener();
                ThroughputPublisher& m_up;
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);

            private:

                DataPubListener& operator=(const DataPubListener&);
        } m_DataPubListener;

        class CommandSubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                CommandSubListener(ThroughputPublisher& up);
                virtual ~CommandSubListener();
                ThroughputPublisher& m_up;
                void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);

            private:

                CommandSubListener& operator=(const CommandSubListener&);
        } m_CommandSubListener;

        class CommandPubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                CommandPubListener(ThroughputPublisher& up);
                virtual ~CommandPubListener();
                ThroughputPublisher& m_up;
                void onPublicationMatched(eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info);

            private:

                CommandPubListener& operator=(const CommandPubListener&);
        } m_CommandPubListener;


        bool ready;

        void run(uint32_t test_time, uint32_t recovery_time_ms, int demand, int msg_size);
        bool test(uint32_t test_time, uint32_t recovery_time_ms, uint32_t demand, uint32_t size);
        std::vector<TroughputResults> m_timeStats;
        ThroughputCommandDataType throuputcommand_t;

        bool loadDemandsPayload();
        std::map<uint32_t,std::vector<uint32_t>> m_demand_payload;

        std::string m_file_name;
        bool m_export_csv;
        std::stringstream output_file;
        uint32_t payload;
        bool reliable_;
        std::string m_sXMLConfigFile;
        std::string m_sExportPrefix;
        //bool dynamic_data = false;
        int m_forced_domain;
        // Static Data
        ThroughputDataType latency_t;
        ThroughputType *latency;
        // Dynamic Data
        //eprosima::fastrtps::types::DynamicData* m_DynData;
        //eprosima::fastrtps::types::DynamicPubSubType m_DynType;
        //eprosima::fastrtps::types::DynamicType_ptr m_pDynType;
        //eprosima::fastrtps::PublisherAttributes pubAttr;
};



#endif /* THROUGHPUTPUBLISHER_H_ */
