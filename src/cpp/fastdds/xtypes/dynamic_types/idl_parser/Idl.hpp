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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDL_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDL_HPP

#include "IdlParser.hpp"

#include <sstream>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/// \brief Parse IDL string input, conforming to IDL specification V4.2.
/// \param[in] idl An IDL string to parse into DynamicTypes
/// \return A Context object with data related to the parse output
inline Context parse(
        const std::string& idl)
{
    return Parser::instance()->parse_string(idl);
}

/// \brief Parse IDL string input with an existent context, conforming to IDL specification V4.2.
/// \param[in] idl An IDL string to parse into DynamicTypes
/// \param[in] context An existent Context object
/// \return A Context object with data related to the parse output
inline Context& parse(
        const std::string& idl,
        Context& context)
{
    Parser::instance()->parse_string(idl, context);
    return context;
}

/// \brief Parse IDL file input, conforming to IDL specification V4.2.
/// \param[in] idl_file Path to the IDL file
/// \return A Context object with data related to the parse output
inline Context parse_file(
        const std::string& idl_file)
{
    return Parser::instance()->parse_file(idl_file);
}

/// \brief Parse IDL file input with an existent context, conforming to IDL specification V4.2.
/// \param[in] idl_file Path to the IDL file
/// \param[in] context An existent Context object
/// \return A Context object with data related to the parse output
inline Context& parse_file(
        const std::string& idl_file,
        Context& context)
{
    Parser::instance()->parse_file(idl_file, context);
    return context;
}

/// \brief Parse IDL file input and save the DynamicTypeBuilder object corresponding to the given target type name,
/// conforming to IDL specification V4.2.
/// \param[in] idl_file Path to the IDL file
/// \param[in] type_name Fully qualified target type name to load from the IDL file
/// \param[in] include_paths A collection of directories to search for additional type description
/// \return A Context object with data related to the parse output
inline Context parse_file(
        const std::string& idl_file,
        const std::string& type_name,
        const IncludePathSeq& include_paths,
        const std::string& preprocessor)
{
    return Parser::instance()->parse_file(idl_file, type_name, include_paths, preprocessor);
}

/// \brief Preprocess IDL file.
/// \param[in] idl_file Path to the IDL file
/// \param[in] includes A collection of directories to search for additional type description
/// \return Preprocessed IDL string
inline std::string preprocess(
        const std::string& idl_file,
        const std::vector<std::string>& includes)
{
    return Parser::preprocess(idl_file, includes);
}

} //namespace idlparser
} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif //FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_IDL_HPP
