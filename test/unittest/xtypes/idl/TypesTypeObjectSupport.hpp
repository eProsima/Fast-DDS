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
 * @file TypesTypeObjectSupport.hpp
 * Header file containing the API required to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef _FAST_DDS_GENERATED_TYPES_TYPE_OBJECT_SUPPORT_HPP_
#define _FAST_DDS_GENERATED_TYPES_TYPE_OBJECT_SUPPORT_HPP_


#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

/**
 * @brief Register every TypeObject representation defined in the IDL file in Fast DDS TypeObjectRegistry.
 */
eProsima_user_DllExport void register_Types_type_objects();

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * @brief Register MyEnumStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MyEnumStruct_type_identifier();
/**
 * @brief Register MyBadEnumStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MyBadEnumStruct_type_identifier();
/**
 * @brief Register MyAliasEnumStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MyAliasEnumStruct_type_identifier();
/**
 * @brief Register BasicStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_BasicStruct_type_identifier();
/**
 * @brief Register BasicNamesStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_BasicNamesStruct_type_identifier();
/**
 * @brief Register BasicBadStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_BasicBadStruct_type_identifier();
/**
 * @brief Register BasicWideStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_BasicWideStruct_type_identifier();
/**
 * @brief Register BadBasicWideStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_BadBasicWideStruct_type_identifier();
/**
 * @brief Register StringStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_StringStruct_type_identifier();
/**
 * @brief Register LargeStringStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_LargeStringStruct_type_identifier();
/**
 * @brief Register WStringStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_WStringStruct_type_identifier();
/**
 * @brief Register LargeWStringStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_LargeWStringStruct_type_identifier();
/**
 * @brief Register ArrayStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_ArrayStruct_type_identifier();
/**
 * @brief Register ArrayStructEqual related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_ArrayStructEqual_type_identifier();
/**
 * @brief Register ArrayBadStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_ArrayBadStruct_type_identifier();
/**
 * @brief Register ArrayDimensionsStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_ArrayDimensionsStruct_type_identifier();
/**
 * @brief Register ArraySizeStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_ArraySizeStruct_type_identifier();
/**
 * @brief Register SequenceStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceStruct_type_identifier();
/**
 * @brief Register SequenceStructEqual related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceStructEqual_type_identifier();
/**
 * @brief Register SequenceBadStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceBadStruct_type_identifier();
/**
 * @brief Register SequenceBoundsStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceBoundsStruct_type_identifier();
/**
 * @brief Register SequenceSequenceStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceSequenceStruct_type_identifier();
/**
 * @brief Register SequenceSequenceBoundsStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SequenceSequenceBoundsStruct_type_identifier();
/**
 * @brief Register MapStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapStruct_type_identifier();
/**
 * @brief Register MapStructEqual related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapStructEqual_type_identifier();
/**
 * @brief Register MapBadKeyStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapBadKeyStruct_type_identifier();
/**
 * @brief Register MapBadElemStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapBadElemStruct_type_identifier();
/**
 * @brief Register MapBoundsStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapBoundsStruct_type_identifier();
/**
 * @brief Register MapMapStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapMapStruct_type_identifier();
/**
 * @brief Register MapMapBoundsStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_MapMapBoundsStruct_type_identifier();
/**
 * @brief Register SimpleUnion related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleUnion_type_identifier();
/**
 * @brief Register SimpleUnionNames related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleUnionNames_type_identifier();
/**
 * @brief Register SimpleTypeUnion related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleTypeUnion_type_identifier();
/**
 * @brief Register SimpleBadUnion related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleBadUnion_type_identifier();
/**
 * @brief Register SimpleBadDiscUnion related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleBadDiscUnion_type_identifier();
/**
 * @brief Register SimpleUnionStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleUnionStruct_type_identifier();
/**
 * @brief Register SimpleUnionStructEqual related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleUnionStructEqual_type_identifier();
/**
 * @brief Register SimpleUnionNamesStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleUnionNamesStruct_type_identifier();
/**
 * @brief Register SimpleTypeUnionStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleTypeUnionStruct_type_identifier();
/**
 * @brief Register SimpleBadUnionStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimpleBadUnionStruct_type_identifier();
/**
 * @brief Register SimplBadDiscUnionStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_SimplBadDiscUnionStruct_type_identifier();


#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // _FAST_DDS_GENERATED_TYPES_TYPE_OBJECT_SUPPORT_HPP_