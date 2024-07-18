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

#ifndef RTPS_BUILTIN_DATA__PROXYDATACONVERTERS_HPP
#define RTPS_BUILTIN_DATA__PROXYDATACONVERTERS_HPP

namespace eprosima {
namespace fastdds {
namespace rtps {

struct ParticipantBuiltinTopicData;
class ParticipantProxyData;
struct PublicationBuiltinTopicData;
class ReaderProxyData;
struct SubscriptionBuiltinTopicData;
class WriterProxyData;

/**
 * Convert a ParticipantProxyData to a ParticipantBuiltinTopicData.
 *
 * @param [in]  proxy_data    ParticipantProxyData to convert.
 * @param [out] builtin_data  ParticipantBuiltinTopicData to fill.
 */
void from_proxy_to_builtin(
        const ParticipantProxyData& proxy_data,
        ParticipantBuiltinTopicData& builtin_data);

/**
 * Convert a ReaderProxyData to a SubscriptionBuiltinTopicData.
 *
 * @param [in]  proxy_data    ReaderProxyData to convert.
 * @param [out] builtin_data  SubscriptionBuiltinTopicData to fill.
 */
void from_proxy_to_builtin(
        const ReaderProxyData& proxy_data,
        SubscriptionBuiltinTopicData& builtin_data);

/**
 * Convert a WriterProxyData to a PublicationBuiltinTopicData.
 *
 * @param [in]  proxy_data    WriterProxyData to convert.
 * @param [out] builtin_data  PublicationBuiltinTopicData to fill.
 */
void from_proxy_to_builtin(
        const WriterProxyData& proxy_data,
        PublicationBuiltinTopicData& builtin_data);

/**
 * Convert a PublicationBuiltinTopicData to a WriterProxyData.
 *
 * @param [in]   builtin_data  PublicationBuiltinTopicData to convert.
 * @param [out]  proxy_data    WriterProxyData to fill.
 */
void from_builtin_to_proxy(
        const PublicationBuiltinTopicData& proxy_data,
        WriterProxyData& builtin_data);

/**
 * Convert a SubscriptionBuiltinTopicData to a ReaderProxyData.
 *
 * @param [in]   builtin_data  SubscriptionBuiltinTopicData to convert.
 * @param [out]  proxy_data    ReaderProxyData to fill.
 */
void from_builtin_to_proxy(
        const SubscriptionBuiltinTopicData& builtin_data,
        ReaderProxyData& proxy_data);

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // RTPS_BUILTIN_DATA__PROXYDATACONVERTERS_HPP
