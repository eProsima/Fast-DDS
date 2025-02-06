// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_DDS_RPC__REQUESTER_HPP
#define FASTDDS_DDS_RPC__REQUESTER_HPP

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/rpc/RPCEntity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class LoanableCollection;
class SampleInfo;

} // namespace dds

namespace dds {
namespace rpc {

class RequestInfo;

/**
 * @brief Base class for a Requester in the RPC communication
 */
class Requester : public RPCEntity
{

public:

    /**
     * @brief Destructor
     */
    virtual ~Requester(){};

    /**
     * @brief Returns the name of the service to which the requester belongs
     */
    virtual const std::string& get_service_name() const = 0;

    /**
     * @brief Send a request message
     * 
     * @param data Data to send
     * @param info Information about the request sample. This information is used to match the request with the reply through the SampleIdentity
     * @return RETCODE_OK if the reply was sent successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t send_request(
            void* data,
            RequestInfo& info) = 0;

    /**
     * @brief Take a reply message from the Requester DataReader's history.
     * 
     * @param data Data to receive the reply
     * @param info Information about the reply sample
     * @return RETCODE_OK if the reply was taken successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t take_reply(
            void* data,
            SampleInfo& info) = 0;

    /**
     * @brief Take all reply messages stored in the Requester DataReader's history.
     * @note This method does not allow to take only the samples associated to a given request. User must implement a zero-copy solution to link request and reply samples.
     * 
     * @param data Data to receive the replies
     * @param info Information about the reply samples
     * @return RETCODE_OK if the replies were taken successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t take_reply(
            LoanableCollection& data,
            LoanableSequence<SampleInfo>& info) = 0;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__REQUESTER_HPP
