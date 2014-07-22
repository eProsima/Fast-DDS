/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file doxygen_modules.h
 */

#ifndef RTPS_DOXYGEN_MODULES_H_
#define RTPS_DOXYGEN_MODULES_H_

//Description of doxygen modules, not used in actual code.

 /*!
 * @defgroup EPROSIMARTPSAPIREFERENCE eProsima RTPS API Reference
 * @brief eProsima RTPS API grouped in modules.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

 /*!
 * @defgroup RTPSMODULE RTPS
 * @ingroup EPROSIMARTPSAPIREFERENCE
 * @brief RTPS API
 * This is an implementation of the RTPS communication standard defined by the OMG.
 * The DDS - Public API provided is provided to the users to create applications and to manage them.
 */

#endif
/*!
 * @defgroup DDSMODULE DDS - Public API
 * @ingroup EPROSIMARTPSAPIREFERENCE
 * @brief DDS Public API
 * This Module contains the Public API for this library.
 */

/** @defgroup ATTRIBUTESMODULE Attributes Module.
 * @ingroup DDSMODULE
 * @brief Attributes class used to define the public entities that the user should use to control this library.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC


/** @defgroup COMMONMODULE Common Module.
 * @ingroup RTPSMODULE
 * Common structures used by multiple elements.
 */

/** @defgroup PARAMETERMODULE Qos Module
 * @ingroup COMMONMODULE
 * All the different Qos and parameters are included here.
 */

/** @defgroup WRITERMODULE Writer Module
 * @ingroup RTPSMODULE
 * This module contains all classes and methods associated with RTPSWriter and its specifications, as well as other necessary classes.
 */

/** @defgroup READERMODULE Reader Module
 * @ingroup RTPSMODULE
 * This module contains all classes and methods associated with RTPSReader and its specifications, as well as other necessary classes.
 */

/** @defgroup MANAGEMENTMODULE Management Module
 * @ingroup RTPSMODULE
 * This module contains classes and methods associated with the management of all other objects. The most important ones
 * are the communication (ResourceSend and ResourceListen) and event (ResourceEvent) resources.
 */

/** @defgroup DISCOVERYMODULE Discovery Module
 * @ingroup MANAGEMENTMODULE
 * This module contains the classes associated with the Discovery Protocols.
 */

/** @defgroup LIVELINESSMODULE Liveliness Module
 * @ingroup MANAGEMENTMODULE
 * This module contains the classes associated with the Writer Liveliness Protocols.
 */



#endif

/** @defgroup UTILITIESMODULE Shared Utilities
 * @ingroup EPROSIMARTPSAPIREFERENCE
 * Shared utilities that can be used by one or more classes in different modules. They are not strictly part of the RTPS implementation
 * but very useful to implement different functionalities.
 */



#endif /* RTPS_DOXYGEN_MODULES_H_ */
