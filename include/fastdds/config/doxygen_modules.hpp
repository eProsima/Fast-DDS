// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file doxygen_modules.hpp
 */

#ifndef FASTDDS_CONFIG__DOXYGEN_MODULES_HPP
#define FASTDDS_CONFIG__DOXYGEN_MODULES_HPP

#include <fastdds/config.hpp>

//Description of doxygen modules, not used in actual code.

/*!
 * @defgroup FASTDDS_GENERAL_API eProsima Fast DDS API Reference
 * @brief eProsima Fast DDS API grouped in modules.
 */

/*!
 * @defgroup RTPS_MODULE RTPS
 * @ingroup FASTDDS_GENERAL_API
 * @brief RTPS API
 * This is an implementation of the RTPS communication standard defined by the OMG.
 */

/*!
 * @defgroup FASTDDS_MODULE DDS Public API
 * @ingroup FASTDDS_GENERAL_API
 * @brief DDS Public API
 * This Module contains the DDS Layer defined by the OMG.
 */

/*!
 * @defgroup dds_xtypes_typeobject DDS TypeObject API
 * @ingroup FASTDDS_MODULE
 */

/*!
 * @defgroup dynamic_language_binding DDS Dynamic Types API
 * @ingroup FASTDDS_MODULE
 */

/** @defgroup FASTDDS_QOS_MODULE Fast DDS Quality of Service (QoS) Module.
 * @ingroup FASTDDS_MODULE
 * @brief QOS class used to define the public entities that the user should use to control this library.
 */

/** @defgroup DEADLINE_MODULE Deadline Module
 * @ingroup FASTDDS_QOS_MODULE
 * This module contains the classes associated with the DEADLINE QoS.
 */

/** @defgroup RTPS_ATTRIBUTES_MODULE RTPS Attributes Module.
 * @ingroup RTPS_MODULE
 * @brief Attributes class used to define the public entities that the user should use to control this library.
 */

/** @defgroup FASTDDS_ATTRIBUTES_MODULE High-level Attributes Module.
 * @ingroup FASTDDS_MODULE
 */

/** @defgroup COMMON_MODULE Common Module.
 * @ingroup RTPS_MODULE
 * Common structures used by multiple elements.
 */

/** @defgroup NETWORK_MODULE Network Module
 * @ingroup RTPS_MODULE
 * Includes the elements necessary to interface between the transport layer and the Fast DDS library.
 */

/** @defgroup TRANSPORT_MODULE Transport Module.
 * @ingroup COMMON_MODULE
 * Built in and user defined transport layer implementations.
 */

/** @defgroup WRITER_MODULE Writer Module
 * @ingroup RTPS_MODULE
 * This module contains all classes and methods associated with RTPSWriter and its specifications, as well as other necessary classes.
 */

/** @defgroup READER_MODULE Reader Module
 * @ingroup RTPS_MODULE
 * This module contains all classes and methods associated with RTPSReader and its specifications, as well as other necessary classes.
 */

/** @defgroup TYPES_MODULE Contains the builtin generated types
 * @namespace eprosima::fastdds::types
 * @ingroup FASTDDS_MODULE
 */

/** @defgroup XMLPARSER_MODULE Contains all the modules related with the XMLParser
 * @namespace eprosima::fastdds::xmlparser
 * @ingroup FASTDDS_MODULE
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/** @defgroup PARAMETER_MODULE Qos Module
 * @ingroup COMMON_MODULE
 * All QoS policies and parameters are included here.
 */

/** @defgroup MANAGEMENT_MODULE Management Module
 * @ingroup FASTDDS_GENERAL_API
 * This module contains classes and methods associated with the management of all other objects.
 * The most important ones are the communication (ResourceSend and ResourceListen) and event (ResourceEvent) resources.
 */

/** @defgroup BUILTIN_MODULE Builtin Protocols Module
 * @ingroup MANAGEMENT_MODULE
 * This module contains the general Builtin Protocols.
 */

/** @defgroup DISCOVERY_MODULE Discovery Module
 * @ingroup MANAGEMENT_MODULE
 * This module contains the classes associated with the Discovery Protocols.
 */

/** @defgroup LIVELINESS_MODULE Liveliness Module
 * @ingroup MANAGEMENT_MODULE
 * This module contains the classes associated with the Writer Liveliness Protocols.
 */

/** @defgroup SECURITY_MODULE Security Module
 * @ingroup MANAGEMENT_MODULE
 * This module contains the classes associated with DDS Security (see https://www.omg.org/spec/DDS-SECURITY/)
 */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#ifdef FASTDDS_STATISTICS
/** @defgroup STATISTICS_MODULE Statistics Module
 * @ingroup FASTDDS_GENERAL_API
 * This module contains the classes associated with the Statistics Protocols.
 */
#endif // ifdef FASTDDS_STATISTICS

/** @defgroup UTILITIES_MODULE Shared Utilities
 * @ingroup FASTDDS_GENERAL_API
 * Shared utilities that can be used by one or more classes in different modules.
 * They are not strictly part of the RTPS implementation but very useful to implement different functionalities.
 */

/**
 * @namespace eprosima eProsima namespace.
 * @ingroup FASTDDS_GENERAL_API
 */

/**
 * @namespace eprosima::fastdds::rtps Contains the RTPS protocol implementation
 * @ingroup RTPS_MODULE
 */

/**
 * @namespace eprosima::fastdds::rtps::TimeConv Auxiliary methods to convert to Time_t to more manageable types.
 *  @ingroup UTILITIES_MODULE
 */

#endif // FASTDDS_CONFIG__DOXYGEN_MODULES_HPP
