// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file EDPClient.h
 *
 */

#ifndef _FASTDDS_RTPS_EDPCLIENT_H_
#define _FASTDDS_RTPS_EDPCLIENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <rtps/builtin/discovery/endpoint/EDPSimple.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class EDPClient, extends the EDPSimple functionality to accommodate client side needs
 *@ingroup DISCOVERY_MODULE
 */
class EDPClient : public EDPSimple
{
public:

    /**
     * Constructor.
     * @param p Pointer to the PDP
     * @param part Pointer to the RTPSParticipantImpl
     */
    EDPClient(
            PDP* p,
            RTPSParticipantImpl* part)
        : EDPSimple(p, part)
    {
    }

    /**
     * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
     * @param rtps_reader Pointer to the Reader object.
     * @param rdata Pointer to the ReaderProxyData object.
     * @return true if correct.
     */
    bool process_reader_proxy_data(
            RTPSReader* rtps_reader,
            ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param rtps_writer Pointer to the Writer object.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool process_writer_proxy_data(
            RTPSWriter* rtps_writer,
            WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param rtps_reader Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool remove_reader(
            RTPSReader* rtps_reader) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param rtps_writer Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool remove_writer(
            RTPSWriter* rtps_writer) override;

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_EDPCLIENT_H_ */
