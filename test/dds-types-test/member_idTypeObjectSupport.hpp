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

/*!
 * @file member_idTypeObjectSupport.hpp
 * Header file containing the API required to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen (version: 4.1.0).
 */

#ifndef FAST_DDS_GENERATED__MEMBER_ID_TYPE_OBJECT_SUPPORT_HPP
#define FAST_DDS_GENERATED__MEMBER_ID_TYPE_OBJECT_SUPPORT_HPP

#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>


#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * @brief Register FixId related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_FixId_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register FixHexId related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_FixHexId_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register FixHashidDefault related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_FixHashidDefault_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register FixHashid related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_FixHashid_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register FixMix related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_FixMix_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register AutoidDefault related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_AutoidDefault_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register AutoidSequential related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_AutoidSequential_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register AutoidHash related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_AutoidHash_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register DerivedAutoidDefault related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_DerivedAutoidDefault_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register DerivedEmptyAutoidSequential related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_DerivedEmptyAutoidSequential_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register DerivedAutoidSequential related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_DerivedAutoidSequential_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);
/**
 * @brief Register DerivedAutoidHash related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 *
 * @param[out] type_ids TypeIdentifier of the registered type.
 *             The returned TypeIdentifier corresponds to the complete TypeIdentifier in case of hashed TypeIdentifiers.
 *             Invalid TypeIdentifier is returned in case of error.
 */
eProsima_user_DllExport void register_DerivedAutoidHash_type_identifier(
        eprosima::fastdds::dds::xtypes::TypeIdentifierPair& type_ids);


#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // FAST_DDS_GENERATED__MEMBER_ID_TYPE_OBJECT_SUPPORT_HPP
