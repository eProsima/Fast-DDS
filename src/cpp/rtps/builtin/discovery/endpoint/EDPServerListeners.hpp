// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file EDPServerListeners.hpp
 *
 */

#ifndef EDPSERVERLISTENER2_H_
#define EDPSERVERLISTENER2_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <rtps/builtin/discovery/endpoint/EDPSimpleListeners.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSReader;
struct CacheChange_t;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPServer;
class EDPServer;

/*!
 * Class EDPServerPUBListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerPUBListener : public fastrtps::rtps::EDPBasePUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPServer associated with this listener.
     */
    EDPServerPUBListener(
            EDPServer* sedp);

    virtual ~EDPServerPUBListener()
    {
    }

    //! return the PDPServer
    PDPServer* get_pdp();

    /**
     * Virtual method,
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader,
            const fastrtps::rtps::CacheChange_t* const change) override;

private:

    //!Pointer to the EDPServer
    EDPServer* sedp_;
};

/*!
 * Class EDPServerSUBListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerSUBListener : public fastrtps::rtps::EDPBaseSUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPServer associated with this listener.
     */
    EDPServerSUBListener(
            EDPServer* sedp);

    virtual ~EDPServerSUBListener() = default;

    //! return the PDPServer
    PDPServer* get_pdp();

    /**
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            fastrtps::rtps::RTPSReader* reader,
            const fastrtps::rtps::CacheChange_t* const change) override;

private:

    //!Pointer to the EDPServer
    EDPServer* sedp_;
};

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* EDPSERVERLISTENER2_H_ */
