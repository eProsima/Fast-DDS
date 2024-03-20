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

#ifndef TYPES_IDL_IDL_H_
#define TYPES_IDL_IDL_H_

#include "parser.h"

#include <sstream>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/// \brief Generates the DynamicTypes related to an idl specification.
/// It supports IDL4.2
/// \param[in] idl An IDL specification to parse into DynamicType.
/// \return A Context with the data related to the parse output.
inline Context parse(
        const std::string& idl)
{
    return Parser::instance()->parse_string(idl);
}

/// \brief Same as parse() but it receives an existant context.
/// \param[in/out] context Data related with the parse output.
inline Context& parse(
        const std::string& idl,
        Context& context)
{
    Parser::instance()->parse_string(idl.c_str(), context);
    return context;
}

/// \brief Same as parse() but it receives a path file where the IDL is located.
/// \param[in] idl_file Path to the idl file.
inline Context parse_file(
        const std::string& idl_file)
{
    return Parser::instance()->parse_file(idl_file);
}

/// \brief Same as parse() but it receives a path file where the IDL is located,
/// and an existant context.
inline Context& parse_file(
        const std::string& idl_file,
        Context& context)
{
    Parser::instance()->parse_file(idl_file.c_str(), context);
    return context;
}

/// \brief Preprocess the file and returns the preprocessed idl content.
inline std::string preprocess(
        const std::string& idl_file,
        const std::vector<std::string>& includes)
{
    return Parser::preprocess(idl_file, includes);
}

/// \brief Generates the IDL that represents an StructType
/// \param[in] type StructType to represent into IDL
/// \return An IDL that represents the StructType given.
//inline std::string generate(
//        const StructType& type)
//{
//    return generator::structure(type.name(), type);
//}

/// \brief Generates the IDL that represents a Module
/// \param[in] module Module to represent into IDL
/// \return An IDL that represents the Module given.
//inline std::string generate(
//        const Module& module)
//{
//    return generator::module(module);
//}

} //namespace idlparser
} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif //TYPES_IDL_IDL_H_
