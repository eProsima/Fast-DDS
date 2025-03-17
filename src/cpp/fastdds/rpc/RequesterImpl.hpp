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

#ifndef FASTDDS_RPC__REQUESTERIMPL_HPP
#define FASTDDS_RPC__REQUESTERIMPL_HPP

#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/rpc/Requester.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

class ServiceImpl;

/**
 * @brief Class that represents a the implementation of a requester entity
 */
class RequesterImpl : public Requester
{

public:

    /**
     * @brief Constructor
     * Don't use it directly, use create_service_requester from DomainParticipant instead
     */
    RequesterImpl(
            ServiceImpl* service,
            const RequesterQos& qos);

    /**
     * @brief Destructor
     */
    virtual ~RequesterImpl();

    /**
     * @brief Returns the name of the service to which the requester belongs
     */
    const std::string& get_service_name() const override;

    /**
     * @brief Send a request message to a replier
     *
     * @param data Data to send
     * @param info Information about the request sample. This information is used to match the request with the reply through the SampleIdentity
     * @return RETCODE_OK if the reply was sent successfully or a ReturnCode related to the specific error otherwise
     */
    ReturnCode_t send_request(
            void* data,
            RequestInfo& info) override;

    /**
     * @brief Take a reply message from the Requester DataReader's history.
     *
     * @param data Data to receive the reply
     * @param info Information about the reply sample
     * @return RETCODE_OK if the reply was taken successfully or a ReturnCode related to the specific error otherwise
     */
    ReturnCode_t take_reply(
            void* data,
            RequestInfo& info) override;

    /**
     * @brief Take all reply messages stored in the Requester DataReader's history.
     * @note This method does not allow to take only the samples associated to a given request. User must implement a zero-copy solution to link request and reply samples.
     *
     * @param data Data to receive the replies
     * @param info Information about the reply samples
     * @return RETCODE_OK if the replies were taken successfully or a ReturnCode related to the specific error otherwise
     */
    ReturnCode_t take_reply(
            LoanableCollection& data,
            LoanableSequence<RequestInfo>& info) override;

    /**
     * @brief This operation indicates to the Requester's DataReader that
     * the application is done accessing the collection of Reply @c datas and @c infos obtained by
     * some earlier invocation of @ref take_reply.
     *
     * @param [in,out] data          A LoanableCollection object where the received data samples were obtained from
     *                               an earlier invocation of take_reply on this Requester.
     * @param [in,out] sample        A LoanableSequence where the received request infos were obtained from
     *                               an earlier invocation of take_reply on this Requester.
     */
    ReturnCode_t return_loan(
            LoanableCollection& data,
            LoanableSequence<RequestInfo>& info) override;

    /**
     * @brief Enable the Requester
     */
    ReturnCode_t enable() override;

    /**
     * @brief Disable the Requester
     */
    ReturnCode_t close() override;

    /**
     * @brief Check if the requester is enabled (i.e: all DDS entities are correctly created)
     */
    inline bool is_enabled() const override
    {
        return enabled_;
    }

    // Getters for DDS Endpoints
    inline DataWriter* get_requester_writer() const override
    {
        return requester_writer_;
    }

    inline DataReader* get_requester_reader() const override
    {
        return requester_reader_;
    }

private:

    /**
     * @brief Create required DDS entities to enable communication with the replier
     *
     * @param qos Requester QoS to configure the DDS entities
     *
     * @return RETCODE_OK if all DDS entities were created successfully, RETCODE_ERROR otherwise
     */
    ReturnCode_t create_dds_entities(
            const RequesterQos& qos);

    /**
     * @brief Delete all internal DDS entities
     *
     * @return RETCODE_OK if all entities were deleted successfully, RETCODE_PRECONDITION_NOT_MET
     * if any entity cannot be deleted
     */
    ReturnCode_t delete_contained_entities();

    DataReader* requester_reader_;
    DataWriter* requester_writer_;
    RequesterQos qos_;
    ServiceImpl* service_;
    bool enabled_;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__REQUESTERIMPL_HPP