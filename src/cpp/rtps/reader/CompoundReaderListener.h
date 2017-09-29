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
 * @file ReaderListener.h
 *
 */

#ifndef COMPOUNDREADERLISTENER_H_
#define COMPOUNDREADERLISTENER_H_

#include <fastrtps/rtps/reader/ReaderListener.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

/**
 * Class CompoundReaderListener. To be used by the Built-in protocols to allow a user ReaderListener to provide additional callbacks
 * on events belonging to the built-in protocols. This is default option when using SimpleEDP.
 * @ingroup READER_MODULE
 */
class CompoundReaderListener: public ReaderListener
{
    public:
        CompoundReaderListener():attached_listener(nullptr){};
        virtual ~CompoundReaderListener(){};

        virtual void onReaderMatched(RTPSReader* /*reader*/, MatchingInfo& /*info*/){};
        virtual void onNewCacheChangeAdded(RTPSReader* /*reader*/, const CacheChange_t* const /*change*/){};

        /**
         * Attaches a secondary ReaderListener to this ReaderListener, so both callbacks are executed on a single event.
         * @param secondary_listener to attach
         */
        void attachListener(ReaderListener *secondary_listener);
        /**
         * Detaches any currently used secondary ReaderListener, but does not manage its destruction
         */
        void detachListener();	
        /**
         * Checks if there is currently a secondary ReaderListener attached to this element
         * @return True if there is a reader attached
         */
        bool hasReaderAttached();
        /**
         * Get a pointer to the secondary ReaderListener attached to this ReaderListener, in case there is one
         * @return ReaderListener pointer to the secondary listener
         */
        ReaderListener* getAttachedListener();	

        //! Mutex to ensure exclusive access to the attachedListener
        std::mutex attached_listener_mutex;
        //! Pointer to the secondary ReaderListener, should there be one attached
        ReaderListener* attached_listener;

};

//Namespace enders
}
}
}

#endif /* COMPOUNDREADERLISTENER_H_ */
