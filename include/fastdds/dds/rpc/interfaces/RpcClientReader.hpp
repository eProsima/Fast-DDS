/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file RpcClientReader.hpp
 */

#ifndef FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTREADER_HPP
#define FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTREADER_HPP

#include <fastdds/dds/core/Time_t.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

/**
 * An interface for a client-side RPC reader.
 *
 * Would be used to read replies from a server on an operation with a `@feed` annotated return type.
 */
template<typename T>
class RpcClientReader
{
public:

    /**
     * Destructor.
     */
    virtual ~RpcClientReader() = default;

    /**
     * Blocking read operation.
     *
     * Will block until a reply is available or the replies feed has finished.
     *
     * @param value The value to read the reply into.
     *
     * @return True if a reply was read, false if the feed has finished or has been cancelled.
     *
     * @throw RpcOperationError if the server communicates an error.
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     */
    virtual bool read(
            T& value) = 0;

    /**
     * Blocking read operation with timeout.
     *
     * Will block until a reply is available, the replies feed has finished, or the timeout expires.
     *
     * @param value The value to read the reply into.
     * @param timeout The maximum time to wait for a reply.
     *
     * @return True if a reply was read, false if the feed has finished or has been cancelled.
     *
     * @throw RpcOperationError if the server communicates an error.
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     * @throw RpcTimeoutException if the timeout expires.
     */
    virtual bool read(
            T& value,
            const eprosima::fastdds::dds::Duration_t& timeout) = 0;

    /**
     * Cancel the replies feed.
     *
     * Will tell the server to stop sending replies, and block until the server acknowledges the cancellation.
     * The replies feed would then be finished locally, so all pending read operations will return false.
     *
     * @throw RpcBrokenPipeException if the communication with the server breaks.
     */
    virtual void cancel() = 0;
};

}  // namespace rpc
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_DDS_RPC_INTERFACES__RPCCLIENTREADER_HPP
