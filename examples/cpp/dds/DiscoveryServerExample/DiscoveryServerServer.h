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
 * @file DiscoveryServerServer.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSERVER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSERVER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include "common.h"

/**
 * Class with a partipant configured to function as server in the Discovery Server mechanism
 */
class DiscoveryServer
{
public:

    DiscoveryServer();

    virtual ~DiscoveryServer();

    //! Initialize the server
    bool init(
            const std::string& server_address,
            unsigned short server_port,
            unsigned short server_id,
            TransportKind transport,
            bool has_connection_server,
            const std::string& connection_server_address,
            unsigned short connection_server_port,
            unsigned short connection_server_id);

    //! Run
    void run(
            unsigned int timeout);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    /**
     * Class handling discovery events
     */
    class ServerListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:

        ServerListener()
        {
        }

        ~ServerListener() override
        {
        }

        //! Callback executed when a DomainParticipant is discovered, dropped or removed
        void on_participant_discovery(
                eprosima::fastdds::dds::DomainParticipant* /*participant*/,
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    private:

        using eprosima::fastdds::dds::DomainParticipantListener::on_participant_discovery;
    }
    listener_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};



#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_DISCOVERYSERVERSERVER_H_ */
