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
 * @file InlineQos.h
 */

#ifndef _FASTDDS_RTPS_INLINEQOS_H_
#define _FASTDDS_RTPS_INLINEQOS_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/utils/MetaTuple.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/core/policy/ParameterSerializer.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * Namespace InlineQoS, contains inline methods to serialize/deserialize parameters into/from inline-qos.
   @ingroup COMMON_MODULE
 */
namespace InlineQoS {

namespace detail {

// Append parameters into inline_qos. Variadic function declaration
template <typename... ParametersTs>
inline bool append_parameters(
    CDRMessage_t& inline_qos,
    ParametersTs&&... parameters);

// Append parameters into inline_qos. Variadic function definition. End of recursion -> do nothing
template <>
inline bool append_parameters(
    CDRMessage_t& inline_qos)
{
    (void)inline_qos;

    return true;
}

//! Append parameters into inline_qos. Variadic function definition. Generic case
template <typename ParameterT, typename... ParametersTs>
inline bool append_parameters(
    CDRMessage_t& inline_qos,
    ParameterT&& parameter,
    ParametersTs&&... parameters)
{
    static_assert(!std::is_same<fastdds::dds::Parameter_t, ParameterT>::value && std::is_base_of<fastdds::dds::Parameter_t, ParameterT>::value, "Parameter type must derive from fastdds::dds::Parameter_t");

    // Add parameter to CDR message
    bool success = fastdds::dds::ParameterSerializer<ParameterT>::add_to_cdr_message(parameter, &inline_qos);

    // Keep looping over the rest of the parameters
    success &= detail::append_parameters(inline_qos, std::forward<ParametersTs>(parameters) ...);

    return success;
}


//! Does contain. Read parameter in CDR message and write it into user tuple
template <typename ParameterT, typename ParametersTupleT>
inline
typename std::enable_if< detail_cpp14::TupleContains<ParameterT, ParametersTupleT>::value, bool>::type 
read_parameter(ParametersTupleT& out_parameters, fastrtps::rtps::CDRMessage_t& cdr_message, const uint16_t plength)
{
    static_assert(!std::is_same<fastdds::dds::Parameter_t, ParameterT>::value && std::is_base_of<fastdds::dds::Parameter_t, ParameterT>::value, "Parameter type must derive from fastdds::dds::Parameter_t");

    return fastdds::dds::ParameterSerializer<ParameterT>::read_from_cdr_message(
        std::get< detail_cpp14::TupleIndex<0, ParameterT, ParametersTupleT>::value >(out_parameters),
        &cdr_message,
        plength);
}

//! ParameterT not contained in ParametersTupleT. Do nothing, as user did not request to read this parameter
template <typename ParameterT, typename ParametersTupleT>
inline
typename std::enable_if<!detail_cpp14::TupleContains<ParameterT, ParametersTupleT>::value, bool>::type 
read_parameter(ParametersTupleT& out_parameters, fastrtps::rtps::CDRMessage_t& cdr_message, const uint16_t plength)
{
    (void)out_parameters;
    (void)cdr_message;
    (void)plength;
    return true;
}

} /* namespace detail */

//////// Public API

/**
 * Serialization from a set of parameters to an Inline-QoS
 *
 * @param[inout] inline_qos On which parameters are writen
 * @param[in] parameters To serialize into inline qos
 */
template <typename... ParametersTs>
inline bool append_parameters(
    SerializedPayload_t& inline_qos,
    ParametersTs&&... parameters)
{
    if (inline_qos.max_size - inline_qos.length < detail_cpp14::LengthCount<ParametersTs...>::size) {
        // Not enough space to accomodate incoming parameters, so reserve
        inline_qos.reserve( detail_cpp14::LengthCount<ParametersTs...>::size + inline_qos.length );
    }

    // Wrap as CDRMessage, so that we can use ParameterSerializer interface
    // Internal inline_qos buffer will contain serialized data
    CDRMessage_t cdr_message(inline_qos);

    // Use templated recursion to loop over all parameters
    bool success = detail::append_parameters(cdr_message, std::forward<ParametersTs>(parameters) ... );

    if (success) {
        // Restart the pointer position
        inline_qos.pos = 0;

        // Set new length of just serialized inline_qos
        inline_qos.length = cdr_message.length;
    }

    return success;
}

/**
 * Deserialization from a set of parameters in Inline-QoS into a tuple of parameters
 *
 * @param[in] inline_qos from which parameters are deserialized
 */
template <typename... ParametersTs>
inline std::tuple<ParametersTs...> read_parameters(const SerializedPayload_t& inline_qos)
{
    using ParametersTupleT = std::tuple<ParametersTs...>;
    ParametersTupleT out_params;

    // Wrap as CDRMessage, so that we can use ParameterSerializer interface
    CDRMessage_t cdr_message(inline_qos);

    auto process_fn = [&out_params](CDRMessage_t* msg, const fastdds::dds::ParameterId_t& pid, uint16_t plength) -> bool
    {
        bool result = true;
        switch (pid) {
            case fastdds::dds::PID_PROPERTY_LIST:
                result = detail::read_parameter<fastdds::dds::ParameterPropertyList_t, ParametersTupleT>(
                    out_params,
                    *msg,
                    plength);
                break;
            case fastdds::dds::PID_CONTENT_FILTER_INFO:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_COHERENT_SET:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_DIRECTED_WRITE:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_ORIGINAL_WRITER_INFO:
                result = detail::read_parameter<fastdds::dds::ParameterOriginalWriterInfo_t, ParametersTupleT>(
                    out_params,
                    *msg,
                    plength);
                break;
            case fastdds::dds::PID_GROUP_COHERENT_SET:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_GROUP_SEQ_NUM:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_WRITER_GROUP_INFO:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_SECURE_WRITER_GROUP_INFO:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_KEY_HASH:
                // TODO implement in ParameterSerializer
                break;
            case fastdds::dds::PID_STATUS_INFO:
                // TODO implement in ParameterSerializer
                break;
            default:
                break;
        }
        return result;
    };

    uint32_t processed_size = 0;
    try
    {
        bool ret = fastdds::dds::ParameterList::readParameterListfromCDRMsg(cdr_message, process_fn, false, processed_size);
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }

    // TODO handle ret

    return out_params;
}

} /* namespace InlineQoS */

/**
 * Type-based accessor from a Parameter Tuple. This is not needed in C++14, as std::get<T> does the job.
 *
 * @param[in] parameters Tuple of parameters
 */
template <typename ParameterT, typename ParametersTupleT>
inline ParameterT& get_parameter(ParametersTupleT&& parameters)
{
    // This would be much simpler in C++14
    return std::get< detail_cpp14::TupleIndex<0, ParameterT, detail_cpp14::remove_ref_t<ParametersTupleT> >::value >( parameters );
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_INLINEQOS_H_ */
