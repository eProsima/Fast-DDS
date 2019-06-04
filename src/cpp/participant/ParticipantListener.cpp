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
 * @file ParticipantListener.cpp
 *
 */

#include <fastrtps/participant/ParticipantListener.h>

namespace eprosima {
namespace fastrtps {

ParticipantListener::ParticipantListener() {}

ParticipantListener::~ParticipantListener() {}

void ParticipantListener::onParticipantDiscovery(
        Participant*,
        rtps::ParticipantDiscoveryInfo&&)
{
}

#if HAVE_SECURITY
void ParticipantListener::onParticipantAuthentication(
        Participant*,
        rtps::ParticipantAuthenticationInfo&&)
{
}
#endif

void ParticipantListener::onSubscriberDiscovery(
        Participant*,
        rtps::ReaderDiscoveryInfo&&)
{
}

void ParticipantListener::onPublisherDiscovery(
        Participant*,
        rtps::WriterDiscoveryInfo&&)
{
}

} // namespace fastrtps
} // namespace eprosima

