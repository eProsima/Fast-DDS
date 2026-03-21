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

/**
 * @file RPCTypeObjectSupport.hpp
 */

#ifndef FASTDDS_DDS_RPC__RPCTYPEOBJECTSUPPORT_HPP
#define FASTDDS_DDS_RPC__RPCTYPEOBJECTSUPPORT_HPP

#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * @brief Register RpcException related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcException_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcBrokenPipeException related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcBrokenPipeException_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcStatusCode related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcStatusCode_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcFeedCancelledException related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcFeedCancelledException_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcOperationError related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcOperationError_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RemoteExceptionCode_t related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RemoteExceptionCode_t_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcRemoteException related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcRemoteException_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

/**
 * @brief Register RpcTimeoutException related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
FASTDDS_EXPORTED_API void register_RpcTimeoutException_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_RPC__RPCTYPEOBJECTSUPPORT_HPP
