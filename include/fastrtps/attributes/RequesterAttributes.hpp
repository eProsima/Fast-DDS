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
 * @file RequesterAttributes.hpp
 */

#ifndef REQUESTERATTRIBUTES_HPP_
#define REQUESTERATTRIBUTES_HPP_

#include "PublisherAttributes.h"
#include "SubscriberAttributes.h"

namespace eprosima {
namespace fastrtps {

class RequesterAttributes
{
public:
    RequesterAttributes() = default;

    bool operator==(const RequesterAttributes& b) const
    {
        return (this->service_name == b.service_name) &&
               (this->request_topic_name == b.request_topic_name) &&
               (this->reply_topic_name == b.reply_topic_name) &&
               (this->publisher == b.publisher) &&
               (this->subscriber == b.subscriber);
    }

    std::string service_name;
    std::string request_type;
    std::string reply_type;
    std::string request_topic_name;
    std::string reply_topic_name;
    PublisherAttributes publisher;
    SubscriberAttributes subscriber;
};


} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* REQUESTERATTRIBUTES_HPP_ */