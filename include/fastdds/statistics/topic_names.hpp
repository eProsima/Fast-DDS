// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file topic_names.hpp
 */

#ifndef _FASTDDS_STATISTICS_TOPIC_NAMES_HPP_
#define _FASTDDS_STATISTICS_TOPIC_NAMES_HPP_

namespace eprosima {
namespace fastdds {
namespace statistics {

//! Statistic topic that reports the write-to-notification latency between any two pairs of matched
//! DataWriter-DataReader histories
constexpr const char* HISTORY_LATENCY_TOPIC = "_fastdds_statistics_history2history_latency";
//! Statistics topic that reports the network latency (message group to message receiver) between any two communicating
//! locators
constexpr const char* NETWORK_LATENCY_TOPIC = "_fastdds_statistics_network_latency";
//! Statistic topic that reports the publication's throughput (amount of data sent) for every DataWriter
constexpr const char* PUBLICATION_THROUGHPUT_TOPIC = "_fastdds_statistics_publication_throughput";
//! Statistics topic that reports the subscription's throughput (amount of data received) for every DataReader
constexpr const char* SUBSCRIPTION_THROUGHPUT_TOPIC = "_fastdds_statistics_subscription_throughput";
//! Statistics topic that reports the number of RTPS packets and bytes sent to each locator
constexpr const char* RTPS_SENT_TOPIC = "_fastdds_statistics_rtps_sent";
//! Statistics topic that reports the number of RTPS packets and bytes that have been lost in the network
constexpr const char* RTPS_LOST_TOPIC = "_fastdds_statistics_rtps_lost";
//! Statistics topic that reports the number of DATA/DATAFRAG sub-messages resent
constexpr const char* RESENT_DATAS_TOPIC = "_fastdds_statistics_resent_datas";
//! Statistics topic that reports the number of HEARTBEATs that each non discovery DataWriter sends
constexpr const char* HEARTBEAT_COUNT_TOPIC = "_fastdds_statistics_heartbeat_count";
//! Statistics topic that reports the number of ACKNACKs that each non discovery DataReader sends
constexpr const char* ACKNACK_COUNT_TOPIC = "_fastdds_statistics_acknack_count";
//! Statistics topic that reports the number of NACKFRAGs that each non discovery DataReader sends
constexpr const char* NACKFRAG_COUNT_TOPIC = "_fastdds_statistics_nackfrag_count";
//! Statistics topic that reports the number of GAPs that each non discovery DataWriter sends
constexpr const char* GAP_COUNT_TOPIC = "_fastdds_statistics_gap_count";
//! Statistics topic that reports the number of DATA/DATAFRAG sub-messages that each non discovery DataWriter sends
constexpr const char* DATA_COUNT_TOPIC = "_fastdds_statistics_data_count";
//! Statistics topic that reports the number of PDP discovery traffic RTPS packets transmitted by each DDS participant
constexpr const char* PDP_PACKETS_TOPIC = "_fastdds_statistics_pdp_packets";
//! Statistics topic that reports the number of EDP discovery traffic RTPS packets transmitted by each DDS participant
constexpr const char* EDP_PACKETS_TOPIC = "_fastdds_statistics_edp_packets";
//! Statistics topic that reports when new entities are discovered
constexpr const char* DISCOVERY_TOPIC = "_fastdds_statistics_discovered_entity";
//! Statistics topic that reports the number of DATA/DATAFRAG sub-messages needed to send a single sample
constexpr const char* SAMPLE_DATAS_TOPIC = "_fastdds_statistics_sample_datas";
//! Statistics topic that reports the host, user and process where the module is running
constexpr const char* PHYSICAL_DATA_TOPIC = "_fastdds_statistics_physical_data";

} // statistics
} // fastdds
} // eprosima

#endif /* _FASTDDS_STATISTICS_TOPIC_NAMES_HPP_ */
