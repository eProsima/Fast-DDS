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

#ifndef _FASTDDS_DDS_LOG_STDOUTCONSUMER_HPP_
#define _FASTDDS_DDS_LOG_STDOUTCONSUMER_HPP_

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class StdoutConsumer : public LogConsumer
{
public:

    virtual ~StdoutConsumer() = default;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // ifndef _FASTDDS_DDS_LOG_STDOUTCONSUMER_HPP_
