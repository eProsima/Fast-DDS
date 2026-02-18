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

/*!
 * @file RPCTypeObjectSupport.cpp
 */

#include <fastdds/dds/rpc/RPCTypeObjectSupport.hpp>

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

using namespace eprosima::fastdds::dds::xtypes;

void register_RpcException_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcException*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcBrokenPipeException_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcBrokenPipeException*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcStatusCode_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcStatusCode*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcFeedCancelledException_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcFeedCancelledException*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcOperationError_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcOperationError*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RemoteExceptionCode_t_type_identifier(
        TypeIdentifierPair& /*type_ids_RemoteExceptionCode_t*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcRemoteException_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcRemoteException*/)
{
    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

void register_RpcTimeoutException_type_identifier(
        TypeIdentifierPair& /*type_ids_RpcTimeoutException*/)
{

    EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
            "RPC is not supported in this Fast DDS version");
}

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima
