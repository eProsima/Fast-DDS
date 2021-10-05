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
 * @file HelloWorldServer.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_HELLOWORLDEXAMPLEDISCOVERYSERVER_HELLOWORLDSERVER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_HELLOWORLDEXAMPLEDISCOVERYSERVER_HELLOWORLDSERVER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>

/**
 * Class with a partipant configured to function as server in the Discovery Server mechanism
 */
class HelloWorldServer
{
public:

    HelloWorldServer();

    virtual ~HelloWorldServer();

    //! Initialize the server
    bool init(
            eprosima::fastdds::rtps::Locator server_address);

    //! Run
    void run();

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};



#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_HELLOWORLDEXAMPLEDISCOVERYSERVER_HELLOWORLDSERVER_H_ */
