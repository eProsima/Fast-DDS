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

#ifndef FASTDDS_RPC__REPLIERIMPL_HPP
#define FASTDDS_RPC__REPLIERIMPL_HPP

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/LoanableCollection.hpp>
#include <fastdds/dds/core/LoanableSequence.hpp>
#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/rpc/Replier.hpp>
#include <fastdds/dds/rpc/ReplierParams.hpp>
#include <fastdds/dds/rpc/RequestInfo.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>

namespace eprosima {
namespace fastdds {
namespace dds{
namespace rpc {

class ServiceImpl;

/**
 * @brief Class that represents the implementation of a replier entity
 */
class ReplierImpl : public Replier
{

friend class ServiceImpl;

    /**
     * @brief Constructor
     * Don't use it directly, use create_service_replier from DomainParticipant instead
     */
    ReplierImpl(
            ServiceImpl* service,
            const ReplierParams& params);

public:

    /**
     * @brief Destructor
     */
    virtual ~ReplierImpl();

    /**
     * @brief Returns the name of the service to which the replier belongs
     */
    const std::string& get_service_name() const override;
    
    /**
     * @brief Send a reply message to a requester
     * 
     * @param data Data to send
     * @param info Information about the reply sample. This information is used to match the reply with the request through the SampleIdentity
     * @return RETCODE_OK if the reply was sent successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t send_reply(
            void* data,
            RequestInfo& info) override;
    
    /**
     * @brief Take a request message from the Replier DataReader's history.
     * 
     * @param data Data to receive the request
     * @param info Information about the request sample
     * @return RETCODE_OK if the request was taken successfully or a ReturnCode related to the specific error otherwise
     */
    virtual ReturnCode_t take_request(
            void* data,
            SampleInfo& info) override;

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
            LoanableSequence<SampleInfo>& info) override;

    /**
     * @brief Check if the replier is valid (i.e: all DDS entities are correctly created)
     */
    inline bool is_valid() const
    {
        return valid_;
    }

    // Getters for DDS Endpoints
    inline const DataWriter* get_replier_writer() const override
    {
        return replier_writer_;
    }

    inline const DataReader* get_replier_reader() const override
    {
        return replier_reader_;
    }

private:

    /**
     * @brief Create required DDS entities to enable communication with the requester
     * 
     * @param params Replier parameters to configure the DDS entities
     * 
     * @return RETCODE_OK if all DDS entities were created successfully, RETCODE_ERROR otherwise
     */
    virtual ReturnCode_t create_dds_entities(const ReplierParams& params);

    DataReader* replier_reader_;
    DataWriter* replier_writer_;
    Publisher* replier_publisher_;
    Subscriber* replier_subscriber_;
    ReplierQos qos_;
    ServiceImpl* service_;
    bool valid_;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__REPLIERIMPL_HPP