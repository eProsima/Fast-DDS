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
 * @file aliasesTypeObjectSupport.hpp
 * Header file containing the API required to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef _FAST_DDS_GENERATED_ALIASES_TYPE_OBJECT_SUPPORT_HPP_
#define _FAST_DDS_GENERATED_ALIASES_TYPE_OBJECT_SUPPORT_HPP_

#include "helpers/basic_inner_typesTypeObjectSupport.hpp"

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
eProsima_user_DllExport void register_aliases_type_objects();

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

/**
 * @brief Register AliasInt16 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasInt16_type_identifier();
/**
 * @brief Register AliasUint16 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasUint16_type_identifier();
/**
 * @brief Register AliasInt32 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasInt32_type_identifier();
/**
 * @brief Register AliasUInt32 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasUInt32_type_identifier();
/**
 * @brief Register AliasInt64 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasInt64_type_identifier();
/**
 * @brief Register AliasUInt64 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasUInt64_type_identifier();
/**
 * @brief Register AliasFloat32 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasFloat32_type_identifier();
/**
 * @brief Register AliasFloat64 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasFloat64_type_identifier();
/**
 * @brief Register AliasFloat128 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasFloat128_type_identifier();
/**
 * @brief Register AliasBool related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasBool_type_identifier();
/**
 * @brief Register AliasOctet related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasOctet_type_identifier();
/**
 * @brief Register AliasChar8 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasChar8_type_identifier();
/**
 * @brief Register AliasChar16 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasChar16_type_identifier();
/**
 * @brief Register AliasString8 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasString8_type_identifier();
/**
 * @brief Register AliasString16 related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasString16_type_identifier();
/**
 * @brief Register AliasEnum related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasEnum_type_identifier();
/**
 * @brief Register AliasBitmask related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasBitmask_type_identifier();
/**
 * @brief Register AliasAlias related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasAlias_type_identifier();
/**
 * @brief Register AliasArray related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasArray_type_identifier();
/**
 * @brief Register AliasMultiArray related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasMultiArray_type_identifier();
/**
 * @brief Register AliasSequence related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasSequence_type_identifier();
/**
 * @brief Register AliasMap related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasMap_type_identifier();
/**
 * @brief Register AliasUnion related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasUnion_type_identifier();
/**
 * @brief Register AliasStruct related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasStruct_type_identifier();
/**
 * @brief Register AliasBitset related TypeIdentifier.
 *        Fully-descriptive TypeIdentifiers are directly registered.
 *        Hash TypeIdentifiers require to fill the TypeObject information and hash it, consequently, the TypeObject is
 *        indirectly registered as well.
 */
void register_AliasBitset_type_identifier();


#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#endif // _FAST_DDS_GENERATED_ALIASES_TYPE_OBJECT_SUPPORT_HPP_