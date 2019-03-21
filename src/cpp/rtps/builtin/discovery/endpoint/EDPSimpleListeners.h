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
 * @file EDPSimpleListeners.h
 *
 */

#ifndef EDPSIMPLELISTENER_H_
#define EDPSIMPLELISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/writer/WriterListener.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class EDPSimple;
class RTPSReader;
struct CacheChange_t;

/*!
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPSimplePUBListener : public ReaderListener, public WriterListener
{
    public:

        /*!
          Constructor
         * @param sedp Pointer to the EDPSimple associated with this listener.
         */
        EDPSimplePUBListener(EDPSimple* sedp) : sedp_(sedp) {}

        virtual ~EDPSimplePUBListener() {}

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

        /**
         * Compute the Key from a CacheChange_t
         * @param change Pointer to the change.
         */
        bool computeKey(CacheChange_t* change);

    private:

        //!Pointer to the EDPSimple
        EDPSimple* sedp_;
};

/*!
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPSimpleSUBListener : public ReaderListener, public WriterListener
{
    public:

        /*!
          Constructor
         * @param sedp Pointer to the EDPSimple associated with this listener.
         */
        EDPSimpleSUBListener(EDPSimple* sedp) : sedp_(sedp){}

        virtual ~EDPSimpleSUBListener(){}
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

        /**
         * @param change
         */
        bool computeKey(CacheChange_t* change);

    private:

        //!Pointer to the EDPSimple
        EDPSimple* sedp_;
};

} /* namespace rtps */
}
} /* namespace eprosima */
#endif
#endif /* EDPSIMPLELISTENER_H_ */
