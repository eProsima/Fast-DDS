/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EPROSIMA_DDS_CORE_DETAIL_CONFORMANCE_HPP_
#define EPROSIMA_DDS_CORE_DETAIL_CONFORMANCE_HPP_

/* The following macros correspond to the compliance "profiles" of the DDS
 * specification. Implementations shall modify this header to indicate their
 * level of support:
 *      - An implementation fully implementing a given profile shall define
 *        the corresponding macro to "FULL".
 *      - An implementation partially supporting a given profile shall define
 *        the corresponding macro to "PARTIAL".
 *      - And implementation with no support for a given profile shall leave
 *        the corresponding macro undefined.
 *
 * DDS Minimum Profile support is required of all DDS implementations;
 * therefore no corresponding macro is provided.
 */

//#define OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT                FULL
//#define OMG_DDS_MULTI_TOPIC_SUPPORT                         FULL
#define OMG_DDS_PERSISTENCE_SUPPORT                         FULL
#define OMG_DDS_OWNERSHIP_SUPPORT                           PARTIAL
#define OMG_DDS_OBJECT_MODEL_SUPPORT                        PARTIAL
#define OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT   PARTIAL
#define OMG_DDS_X_TYPES_DYNAMIC_TYPE_SUPPORT                FULL
//#define OMG_DDS_HAS_PRETTY_PRINT_COUT 1

#endif //EPROSIMA_DDS_CORE_DETAIL_CONFORMANCE_HPP_
