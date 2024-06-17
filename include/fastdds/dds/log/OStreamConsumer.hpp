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

#ifndef FASTDDS_DDS_LOG__OSTREAMCONSUMER_HPP
#define FASTDDS_DDS_LOG__OSTREAMCONSUMER_HPP

#include <iostream>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Log consumer interface for std::ostream objects
 *
 * @file OStreamConsumer.hpp
 */
class OStreamConsumer : public LogConsumer
{
public:

    virtual ~OStreamConsumer() = default;

    /** \internal
     * Called by Log to ask us to consume the Entry.
     * @param Log::Entry to consume.
     */
    FASTDDS_EXPORTED_API void Consume(
            const Log::Entry& entry) override;

protected:

    /** \internal
     * Called by Log consume to get the correct stream
     * @param Log::Entry to consume.
     */
    FASTDDS_EXPORTED_API virtual std::ostream& get_stream(
            const Log::Entry& entry) = 0;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_LOG__OSTREAMCONSUMER_HPP
