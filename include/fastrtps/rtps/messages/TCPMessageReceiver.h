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
 * @file TCPMessageReceiver.h
 */



#ifndef TCP_MESSAGERECEIVER_H_
#define TCP_MESSAGERECEIVER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../common/all_common.h"
#include "../../qos/ParameterList.h"
#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include "fastrtps/transport/tcp/TCPControlMessage.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class TCPSocketInfo;

/**
 * Class TCPMessageReceiver, process the received TCP messages.
 * @ingroup MANAGEMENT_MODULE
 */
class TCPMessageReceiver
{
public:

    TCPMessageReceiver();
    virtual ~TCPMessageReceiver();

    void sendConnectionRequest(TCPSocketInfo* pSocketInfo, const Locator_t &transportLocator);
    void sendOpenLogicalPortRequest(TCPSocketInfo* pSocketInfo, OpenLogicalPortRequest_t &request);
    void sendCheckLogicalPortsRequest(TCPSocketInfo* pSocketInfo, CheckLogicalPortsRequest_t &request);
    void sendKeepAliveRequest(TCPSocketInfo* pSocketInfo, KeepAliveRequest_t &request);
    void sendLogicalPortIsClosedRequest(TCPSocketInfo* pSocketInfo, LogicalPortIsClosedRequest_t &request);

    void processConnectionRequest(TCPSocketInfo* pSocketInfo, const ConnectionRequest_t &request, 
        Locator_t &localLocator);
    void processOpenLogicalPortRequest(TCPSocketInfo* pSocketInfo, const OpenLogicalPortRequest_t &request);
    void processCheckLogicalPortsRequest(TCPSocketInfo* pSocketInfo, const CheckLogicalPortsRequest_t &request);
    void processKeepAliveRequest(TCPSocketInfo* pSocketInfo, const KeepAliveRequest_t &request);
    void processLogicalPortIsClosedRequest(TCPSocketInfo* pSocketInfo, const LogicalPortIsClosedRequest_t &request);

    void processBindConnectionResponse(TCPSocketInfo* pSocketInfo, const BindConnectionResponse_t &response);
    void processCheckLogicalPortsResponse(TCPSocketInfo* pSocketInfo, const CheckLogicalPortsResponse_t &response);
    void processResponse(TCPSocketInfo* pSocketInfo, const ControlProtocolResponseData &response);

private:
    bool sendResponseData(TCPSocketInfo* pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const ControlProtocolResponseData &response);
    bool sendRequestData(TCPSocketInfo* pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const ControlProtocolRequestData &request);
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* TCP_MESSAGERECEIVER_H_ */
