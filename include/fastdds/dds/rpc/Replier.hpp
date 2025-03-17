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

#ifndef FASTDDS_DDS_RPC__REPLIER_HPP
#define FASTDDS_DDS_RPC__REPLIER_HPP

#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/rpc/RPCEntity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class LoanableCollection;
class DataWriter;
class DataReader;

} // namespace dds

namespace dds {
namespace rpc {

/**
 * @brief Base class for a Replier in the RPC communication
 */
class Replier : public RPCEntity
{

public:

    /**
     * @brief Returns the name of the service to which the replier belongs
     */
    virtual const std::string& get_service_name() const = 0;

    /**
     * @brief Send a reply message
     *
     * @param data Data to send
     * @param info Information about the reply sample. This information is used to match the reply with the request through the SampleIdentity
     * @return RETCODE_OK if the reply was sent successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t send_reply(
            void* data,
            const RequestInfo& info) = 0;

    /**
     * @brief Take a request message from the Replier DataReader's history.
     *
     * @param data Data to receive the request
     * @param info Information about the request sample
     * @return RETCODE_OK if the request was taken successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t take_request(
            void* data,
            RequestInfo& info) = 0;

    /**
     * @brief Take all request messages stored in the Replier DataReader's history.
     * @note This method does not allow to take only the samples associated to a given request. User must implement a zero-copy solution to link request and reply samples.
     *
     * @param data Data to receive the request
     * @param info Information about the request sample
     * @return RETCODE_OK if the request was taken successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t take_request(
            LoanableCollection& data,
            LoanableSequence<RequestInfo>& info) = 0;

    /**
     * @brief This operation indicates to the Replier's DataReader that
     * the application is done accessing the collection of Request @c datas and @c infos obtained by
     * some earlier invocation of @ref take_request.
     *
     * @param [in,out] data          A LoanableCollection object where the received data samples were obtained from
     *                               an earlier invocation of take_request on this Replier.
     * @param [in,out] info          A LoanableSequence where the received request infos were obtained from
     *                               an earlier invocation of take_request on this Replier.
     */
    virtual ReturnCode_t return_loan(
            LoanableCollection& data,
            LoanableSequence<RequestInfo>& info) = 0;

    /**
     * @brief Getter for the Replier's DataWriter
     */
    virtual DataWriter* get_replier_writer() const = 0;

    /**
     * @brief Getter for the Replier's DataReader
     */
    virtual DataReader* get_replier_reader() const = 0;

protected:

    /**
     * @brief Destructor
     */
    ~Replier() = default;

};

} // namespace rpc
} // namespace dds
} // namesapce fastdds
} // namespace eprosima


#endif // FASTDDS_DDS_RPC__REPLIER_HPP