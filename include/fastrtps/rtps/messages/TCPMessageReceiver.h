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

    bool CheckTCPControlMessage(TCPSocketInfo* pSocketInfo, octet* buffer, uint32_t bufferSize);

private:
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* TCP_MESSAGERECEIVER_H_ */
