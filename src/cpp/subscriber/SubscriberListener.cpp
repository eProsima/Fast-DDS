// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriberListener.cpp
 */

#include <fastrtps/subscriber/SubscriberListener.h>

namespace eprosima {
namespace fastrtps {

namespace rtps {
class MatchingInfo;
} /* namespace rtps */

class Subscriber;

SubscriberListener::SubscriberListener(){}

SubscriberListener::~SubscriberListener(){}

void SubscriberListener::onNewDataMessage(
        Subscriber*)
{
}

void SubscriberListener::onSubscriptionMatched(
        Subscriber*,
        rtps::MatchingInfo&)
{
}

void SubscriberListener::on_requested_deadline_missed(
        Subscriber*,
        const RequestedDeadlineMissedStatus&)
{
}

void SubscriberListener::on_liveliness_changed(
        Subscriber*,
        const LivelinessChangedStatus&)
{
}


} /* namespace fastrtps */
} /* namespace eprosima */
