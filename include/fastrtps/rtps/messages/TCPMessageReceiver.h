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

    void sendConnectionRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const Locator_t &transportLocator);
    void sendOpenLogicalPortRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, OpenLogicalPortRequest_t &request);
    void sendCheckLogicalPortsRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, CheckLogicalPortsRequest_t &request);
    void sendKeepAliveRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, KeepAliveRequest_t &request);
    void sendLogicalPortIsClosedRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, LogicalPortIsClosedRequest_t &request);

    void processConnectionRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const ConnectionRequest_t &request, 
        Locator_t &localLocator);
    void processOpenLogicalPortRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const OpenLogicalPortRequest_t &request);
    void processCheckLogicalPortsRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const CheckLogicalPortsRequest_t &request);
    void processKeepAliveRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const KeepAliveRequest_t &request);
    void processLogicalPortIsClosedRequest(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const LogicalPortIsClosedRequest_t &request);

    void processBindConnectionResponse(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const BindConnectionResponse_t &response);
    void processCheckLogicalPortsResponse(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const CheckLogicalPortsResponse_t &response);
    void processResponse(std::shared_ptr<TCPSocketInfo> &pSocketInfo, const ControlProtocolResponseData &response);

private:
    bool sendResponseData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const ControlProtocolResponseData &response);
    bool sendRequestData(std::shared_ptr<TCPSocketInfo> &pSocketInfo, 
        const TCPHeader &header, const TCPControlMsgHeader &ctrlHeader,
        const ControlProtocolRequestData &request);
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* TCP_MESSAGERECEIVER_H_ */
