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
 * RTPSReader.cpp
 *
 */
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>

#include <rtps/participant/RTPSParticipantImpl.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

RTPSReader::RTPSReader(
        RTPSParticipantImpl* pimpl,
        const GUID_t& guid,
        const ReaderAttributes& att,
        ReaderHistory* hist)
    : Endpoint(pimpl, guid, att.endpoint)
    , history_(hist)
{
    history_->mp_reader = this;
    history_->mp_mutex = &mp_mutex;
}

RTPSReader::~RTPSReader()
{
    history_->mp_reader = nullptr;
    history_->mp_mutex = nullptr;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
