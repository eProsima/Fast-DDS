/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_CORE_DETAIL_CONFORMANCE_HPP_
#define OSPL_DDS_CORE_DETAIL_CONFORMANCE_HPP_

// Implementation

/**
 * @file
 * @internal
 * @note Values 'set' in this file should be mirrored in etc/doxygen_isocpp2_common.cfg
 * in order to ensure the doxygen documentation includes all supported QoS
 * and features and ting.
 */

#define OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT                FULL
// #define OMG_DDS_MULTI_TOPIC_SUPPORT                         FULL
#define OMG_DDS_PERSISTENCE_SUPPORT                         FULL
#define OMG_DDS_OWNERSHIP_SUPPORT                           FULL
#define OMG_DDS_OBJECT_MODEL_SUPPORT                        FULL
// #define OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT   FULL
#define OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT         FULL

#define OMG_DDS_HAS_PRETTY_PRINT_COUT 1
// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_CONFORMANCE_HPP_ */
