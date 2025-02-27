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

#ifndef FASTDDS_DDS_RPC__SERVICETYPESUPPORT_HPP
#define FASTDDS_DDS_RPC__SERVICETYPESUPPORT_HPP

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

class ServiceTypeSupport
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport() noexcept = default;

    /**
     * @brief Copy Constructor
     *
     * @param service_type Another instance of ServiceTypeSupport
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport(
            const ServiceTypeSupport& service_type) noexcept = default;

    /**
     * @brief Move Constructor
     *
     * @param service_type Another instance of ServiceTypeSupport
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport(
            ServiceTypeSupport&& service_type) noexcept = default;

    /**
     * @brief Copy Assignment
     *
     * @param service_type Another instance of ServiceTypeSupport
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport& operator = (
            const ServiceTypeSupport& service_type) noexcept = default;

    /**
     * @brief Move Assignment
     *
     * @param service_type Another instance of ServiceTypeSupport
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport& operator = (
            ServiceTypeSupport&& service_type) noexcept = default;

    /**
     * @brief ServiceTypeSupport constructor that receives two TypeSupport objects (Request + reply TypeSupports)
     *
     * @param request_type TypeSupport of the request type
     * @param reply_type TypeSupport of the reply type
     */
    FASTDDS_EXPORTED_API ServiceTypeSupport(
            TypeSupport request_type,
            TypeSupport reply_type)
        : request_type_(request_type)
        , reply_type_(reply_type)

    {
    }

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~ServiceTypeSupport() = default;

    /**
     * @brief Registers the service type on a participant
     *
     * @param participant DomainParticipant where the service type is going to be registered
     * @param service_type_name Name of the service type to register
     * @return RETCODE_BAD_PARAMETER if the service name is empty, RETCODE_PRECONDITION_NOT_MET if there is another service with
     * the same name registered on the DomainParticipant and RETCODE_OK if it is registered correctly
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t register_service_type(
            DomainParticipant* participant,
            std::string service_type_name) const;

    /**
     * @brief Returns the TypeSupport of the request type
     */
    FASTDDS_EXPORTED_API const TypeSupport request_type() const
    {
        return request_type_;
    }

    /**
     * @brief Returns the TypeSupport of the reply type
     */
    FASTDDS_EXPORTED_API const TypeSupport reply_type() const
    {
        return reply_type_;
    }

    /**
     * @brief Check if the ServiceTypeSupport object contains empty request/reply types
     */
    FASTDDS_EXPORTED_API inline bool empty_types() const
    {
        return request_type_.empty() || reply_type_.empty();
    }

    FASTDDS_EXPORTED_API bool operator ==(
            const ServiceTypeSupport& type_support)
    {
        TypeSupport request_type = type_support.request_type();
        TypeSupport reply_type = type_support.reply_type();

        return request_type == this->request_type() && reply_type == this->reply_type();
    }

private:

    TypeSupport request_type_;
    TypeSupport reply_type_;

};
} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__SERVICETYPESUPPORT_HPP