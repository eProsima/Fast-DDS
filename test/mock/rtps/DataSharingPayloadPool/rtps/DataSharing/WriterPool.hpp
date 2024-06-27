// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WriterPool.hpp
 */

#ifndef RTPS_DATASHARING_WRITERPOOL_HPP
#define RTPS_DATASHARING_WRITERPOOL_HPP

#include <memory>

#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/DataSharing/DataSharingPayloadPool.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

class WriterPool : public DataSharingPayloadPool
{
public:

    bool init_shared_memory(
            const RTPSWriter* writer,
            const std::string& /*shared_dir*/) override
    {
        writer_guid_ = writer->getGuid();
        return true;
    }

    bool is_initialized() const
    {
        return writer_guid_ != GUID_t::unknown();
    }

    void add_to_shared_history(
            const CacheChange_t* /*cache_change*/)
    {
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_DATASHARING_WRITERPOOL_HPP
