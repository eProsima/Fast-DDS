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

#include <fastdds/rtps/builtin/discovery/endpoint/EDPSimple.h>

namespace eprosima {
namespace fastrtps {
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
      * @param reader Pointer to the Reader object.
      * @param rdata Pointer to the ReaderProxyData object.
      * @return true if correct.
      */
    bool processLocalReaderProxyData(
            RTPSReader* reader,
            ReaderProxyData* rdata) override;
    /**
     * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
     * @param writer Pointer to the Writer object.
     * @param wdata Pointer to the WriterProxyData object.
     * @return true if correct.
     */
    bool processLocalWriterProxyData(
            RTPSWriter* writer,
            WriterProxyData* wdata) override;
    /**
     * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
     * @param R Pointer to the RTPSReader object.
     * @return True if correct.
     */
    bool removeLocalReader(
            RTPSReader*R) override;
    /**
     * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
     * @param W Pointer to the RTPSWriter object.
     * @return True if correct.
     */
    bool removeLocalWriter(
            RTPSWriter*W) override;

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* _FASTDDS_RTPS_EDPCLIENT_H_ */
