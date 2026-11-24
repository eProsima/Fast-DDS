// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Link-seam implementation of RTPSReader: replaces src/cpp/rtps/reader/RTPSReader.cpp. As the real one, it binds the
// history, if any, to the reader and its mutex on construction.

#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>

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
    if (nullptr != history_)
    {
        history_->mp_reader = this;
        history_->mp_mutex = &mp_mutex;
    }
}

RTPSReader::~RTPSReader()
{
    // Unlike the real one, the history is not unbound, as tests may destroy it before the reader.
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
