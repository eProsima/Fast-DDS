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
 * @file HelloWorldPublisher.h
 *
 */

#ifndef HELLOWORLDPUBLISHER_H_
#define HELLOWORLDPUBLISHER_H_

#include "HelloWorldPubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>


#include "HelloWorld.h"

#include <vector>

class HelloWorldPublisher {
    class PubListener :public eprosima::fastrtps::PublisherListener
    {
    public:
        PubListener()
            : n_matched(0)
            , firstConnected(false)
        {};

        ~PubListener() {};

        void onPublicationMatched(
            eprosima::fastrtps::Publisher* pub,
            eprosima::fastrtps::rtps::MatchingInfo& info);

        int n_matched;
        bool firstConnected;
    } listener_;

    HelloWorld hello_;
    eprosima::fastrtps::Participant* participant_;
    eprosima::fastrtps::Publisher* publisher_;
    bool stop_;
    HelloWorldPubSubType type_;

    void runThread(
        uint32_t number,
        long sleep_ms);

public:
    HelloWorldPublisher();

    virtual ~HelloWorldPublisher();

    //!Initialize
    bool init(
        const std::string &wan_ip,
        unsigned short port,
        bool use_tls,
        const std::vector<std::string>& whitelist);

    //!Publish a sample
    bool publish(bool waitForListener = true);

    //!Run for number samples
    void run(
        uint32_t number,
        long sleep_ms);
};



#endif /* HELLOWORLDPUBLISHER_H_ */
