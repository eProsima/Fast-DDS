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

/*
 * @file RTPSWriter.cpp
 *
 */

#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSWriter::RTPSWriter(
        RTPSParticipantImpl* impl,
        const GUID_t& guid,
        const WriterAttributes& att)
    : Endpoint(impl, guid, att.endpoint)
{
}

RTPSWriter::~RTPSWriter()
{
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
