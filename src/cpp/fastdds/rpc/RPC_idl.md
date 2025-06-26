# TypeObject support for RPC types

The code for registering the TypeObject of RPC exception classes has been developed by updating
auto-generated code from the IDL below.

This IDL has not been included in the automatic regeneration script for the following reasons:

* The generated header needs to be a public header, and that requires updating the DLL export
  annotation, as well as adding the corresponding `#include <fastdds/fastdds_dll.hpp>`.
  These changes could be automated, but changes in the generated code might also require changes
  in the automated patch.
* Only the TypeObject support code is necessary.
  The generated classes, type support, and (de)serialization code would need to be removed.
* Changes in the model would be not be frequent.

## IDL model of RPC exception classes

```
module eprosima {
module fastdds {
module dds {
module rpc {

// Base class for RPC exceptions.
@nested @final
struct RpcException
{
    string message;
};

// Exception thrown by the RPC API when the communication with the remote endpoint breaks.
@nested @final
struct RpcBrokenPipeException : RpcException
{
};

// Code used in RpcFeedCancelledException
typedef unsigned long RpcStatusCode;

// Exception thrown by the RPC API when the client cancels an input feed.
@nested @final
struct RpcFeedCancelledException : RpcException
{
    RpcStatusCode reason;
};

// Base class for user defined exceptions.
@nested @final
struct RpcOperationError : RpcException
{
};

/**
 * Enumeration of possible error codes that can be returned by a remote service.
 * Extracted from DDS-RPC v1.0 - 7.5.2 Mapping of Error Codes.
 */
@final
enum RemoteExceptionCode_t
{
    REMOTE_EX_OK,                 // The request was executed successfully.
    REMOTE_EX_UNSUPPORTED,        // Operation is valid but it is unsupported (a.k.a. not implemented).
    REMOTE_EX_INVALID_ARGUMENT,   // The value of a parameter passed has an illegal value.
    REMOTE_EX_OUT_OF_RESOURCES,   // The remote service ran out of resources while processing the request.
    REMOTE_EX_UNKNOWN_OPERATION,  // The operation called is unknown.
    REMOTE_EX_UNKNOWN_EXCEPTION   // A generic, unspecified exception was raised by the service implementation.
};

// Class for exceptions that map to a RpcExceptionCode_t
@nested @final
struct RpcRemoteException : RpcException
{
    RemoteExceptionCode_t code;
};

// Exception thrown by the RPC API when an operation times out.
@nested @final
struct RpcTimeoutException : RpcException
{
};

};  // module rpc
};  // module dds
};  // module fastdds
};  // module eprosima

```
