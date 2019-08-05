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
 * @file EDPServerListeners.h
 *
 */

#ifndef EDPSERVERLISTENER_H_
#define EDPSERVERLISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <rtps/builtin/discovery/endpoint/EDPSimpleListeners.h>


namespace eprosima {
namespace fastrtps {
namespace rtps {

class EDPServer;
class RTPSReader;
struct CacheChange_t;

/*!
 * Class EDPServerPUBListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerPUBListener : public EDPBasePUBListener
{
    public:

        /*!
          Constructor
         * @param sedp Pointer to the EDPServer associated with this listener.
         */
        EDPServerPUBListener(EDPServer* sedp);

        virtual ~EDPServerPUBListener() {}

        /**
         * Virtual method,
         * @param reader
         * @param change
         */
        void onNewCacheChangeAdded(
                RTPSReader* reader,
                const CacheChange_t* const  change) override;


        /*!
         * This method is called when all the readers matched with this Writer acknowledge that a cache
         * change has been received.
         * @param writer Pointer to the RTPSWriter.
         * @param change Pointer to the affected CacheChange_t.
         */
        void onWriterChangeReceivedByAll(
                RTPSWriter* writer,
                CacheChange_t* change) override;

    private:

        //!Pointer to the EDPServer
        EDPServer* sedp_;
};

/*!
 * Class EDPServerSUBListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerSUBListener : public EDPBaseSUBListener
{
    public:

        /*!
          Constructor
         * @param sedp Pointer to the EDPServer associated with this listener.
         */
        EDPServerSUBListener(EDPServer* sedp);

        virtual ~EDPServerSUBListener() = default;
        /**
         * @param reader
         * @param change
         */
        void onNewCacheChangeAdded(
                RTPSReader* reader,
                const CacheChange_t* const change) override;

        /*!
         * This method is called when all the readers matched with this Writer acknowledge that a cache
         * change has been received.
         * @param writer Pointer to the RTPSWriter.
         * @param change Pointer to the affected CacheChange_t.
         */
        void onWriterChangeReceivedByAll(
                RTPSWriter* writer,
                CacheChange_t* change) override;

    private:

        //!Pointer to the EDPServer
        EDPServer* sedp_;
};

} /* namespace rtps */
}
} /* namespace eprosima */
#endif
#endif /* EDPSERVERLISTENER_H_ */
