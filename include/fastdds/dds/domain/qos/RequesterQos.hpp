// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RequesterQos.hpp
 */

#ifndef _FASTDDS_REQUESTERQOS_HPP_
#define _FASTDDS_REQUESTERQOS_HPP_

#include <string>

#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class RequesterQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API RequesterQos() = default;

    /**
     * @brief Equal comparison operator
     */
    FASTDDS_EXPORTED_API bool operator ==(
            const RequesterQos& b) const
    {
        return (this->service_name == b.service_name) &&
               (this->request_topic_name == b.request_topic_name) &&
               (this->reply_topic_name == b.reply_topic_name) &&
               (this->writer_qos == b.writer_qos) &&
               (this->reader_qos == b.reader_qos);
    }

    //! Service name
    std::string service_name;

    //! Request type
    std::string request_type;

    //! Reply type
    std::string reply_type;

    //! Request topic name
    std::string request_topic_name;

    //! Reply topic name
    std::string reply_topic_name;

    //! DataWriter QoS for the request writer
    DataWriterQos writer_qos;

    //! DataReader QoS for the request reader
    DataReaderQos reader_qos;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_REQUESTERQOS_HPP_ */
