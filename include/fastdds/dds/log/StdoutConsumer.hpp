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

#ifndef FASTDDS_DDS_LOG__STDOUTCONSUMER_HPP
#define FASTDDS_DDS_LOG__STDOUTCONSUMER_HPP

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/OStreamConsumer.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class StdoutConsumer : public OStreamConsumer
{
public:

    virtual ~StdoutConsumer() = default;

private:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param Log::Entry to consume.
     */
    FASTDDS_EXPORTED_API virtual std::ostream& get_stream(
            const Log::Entry& entry) override;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_LOG__STDOUTCONSUMER_HPP
