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
 * @file Server.hpp
 */

#ifndef _FASTDDS_DDS_RPC_SERVER_HPP_
#define _FASTDDS_DDS_RPC_SERVER_HPP_

#include <functional>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief The Server class represents an RPC server.
 *
 * This class provides functionality for creating an RPC server that can handle incoming requests
 * and send back responses. It is templated on the types of the request and response data.
 *
 * @tparam RequestTypeSupport The type support class for the request data type.
 * @tparam ResponseTypeSupport The type support class for the response data type.
 */
template<
    typename RequestTypeSupport,
    typename ResponseTypeSupport
    >
class Server : protected DataReaderListener
{
public:

    using RequestType = typename RequestTypeSupport::type;
    using ResponseType = typename ResponseTypeSupport::type;
    using ServiceProcedure = std::function<ResponseType (const RequestType&)>;

    /*
     * TODO(eduponz): Add ctor overloads for instantiating Servers in different ways, e.g.:
     *    1. Only receive a Server name
     *    2. Do not receive a participant (would make the current ctor suitable to receive a ref)
     *    3. Set different QoS for the request and response DataReader and DataWriter respectively
     *    4. Optionally inject pools for reusing request and response samples.
     *    5. Provide the type support instances to avoid creating them in the ctor.
     */

    /**
     * @brief Constructs a Server object.
     *
     * @param participant The DomainParticipant object to use for communication. It is used to
     *                    create the necessary entities for the server. The listener is not
     *                    overriden, with the except of the on_data_available method, which is
     *                    overriden by the Server class, which in turn acts as a DataReaderListener
     *                    for the request DataReader. Consequently, the data_available bit of the
     *                    provided DomainParticipant status mask is disabled, as well as the
     *                    data_on_readers one. If set to nullptr, a DomainParticipant is created.
     * @param request_topic_name The name of the topic for receiving requests.
     * @param response_topic_name The name of the topic for sending responses.
     * @param max_ack_wait The maximum time to wait for acknowledgment of the response before
     *                     processing the next request.
     * @param service_procedure The service procedure to handle incoming requests.
     */
    RTPS_DllAPI Server(
            DomainParticipant* participant,
            const std::string& request_topic_name,
            const std::string& response_topic_name,
            Duration_t max_ack_wait,
            ServiceProcedure service_procedure);

    /**
     * @brief Runs the server.
     *
     * @pre This function can only be called once.
     * @pre This function cannot be called after stop has been called.
     *
     * This function starts the server and processes requests asynchronously by processing a
     * reception queue. It is a blocking function that will not return until the server is stopped,
     * and it that sense it is meant to be executed in a user-provided execution context (e.g. on
     * its onw thread). This function executes the provided service procedure for each incoming
     * request, and it is used to send the responses back to the clients.
     */
    RTPS_DllAPI void run();

    /**
     * @brief Stops the server.
     *
     * This function stops the server and terminates any ongoing operations.
     * If deletes all the server's entities and releases the resources, so the server instance
     * cannot be used after this function is called.
     *
     * @return true if the server was successfully stopped, false otherwise.
     */
    RTPS_DllAPI bool stop();

protected:

    /**
     * @brief Callback function for handling incoming data.
     *
     * This function is called when new data becomes available on the DataReader.
     * It is responsible for processing the incoming requests.
     *
     * @param reader The DataReader object that received the data.
     */
    void on_data_available(
            DataReader* reader) override;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_RPC_SERVER_HPP_
