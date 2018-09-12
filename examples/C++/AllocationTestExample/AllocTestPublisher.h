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
 * @file AllocTestPublisher.h
 *
 */

#ifndef ALLOCTESTPUBLISHER_H_
#define ALLOCTESTPUBLISHER_H_

#include "AllocTestTypePubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>


#include "AllocTestType.h"

class AllocTestPublisher {
public:
    AllocTestPublisher();
    virtual ~AllocTestPublisher();
    //!Initialize
    bool init(const char* profile);
    //!Publish a sample
    bool publish();
    //!Run for number samples
    void run(uint32_t number);
private:
    AllocTestTypePubSubType m_type;
    AllocTestType m_data;
    eprosima::fastrtps::Participant* mp_participant;
    eprosima::fastrtps::Publisher* mp_publisher;
    class PubListener:public eprosima::fastrtps::PublisherListener
    {
    public:
        PubListener():n_matched(0){};
        ~PubListener(){};
        void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info);
        int n_matched;
    }m_listener;
};



#endif /* ALLOCTESTPUBLISHER_H_ */
