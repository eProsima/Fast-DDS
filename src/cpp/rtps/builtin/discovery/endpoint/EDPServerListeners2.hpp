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
 * @file EDPServerListeners2.hpp
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

// To be eventually removed together with eprosima::fastrtps
namespace aux = ::eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace rtps {

class PDPServer2;
class EDPServer2;

/*!
 * Class EDPServerPUBListener2, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerPUBListener2 : public aux::EDPBasePUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPServer associated with this listener.
     */
    EDPServerPUBListener2(
            EDPServer2* sedp);

    virtual ~EDPServerPUBListener2()
    {
    }

    //! return the PDPServer2
    PDPServer2* get_pdp();

    /**
     * Virtual method,
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            aux::RTPSReader* reader,
            const aux::CacheChange_t* const change) override;

private:

    //!Pointer to the EDPServer
    EDPServer2* sedp_;
};

/*!
 * Class EDPServerSUBListener2, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerSUBListener2 : public aux::EDPBaseSUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPServer associated with this listener.
     */
    EDPServerSUBListener2(
            EDPServer2* sedp);

    virtual ~EDPServerSUBListener2() = default;

    //! return the PDPServer2
    PDPServer2* get_pdp();

    /**
     * @param reader
     * @param change
     */
    void onNewCacheChangeAdded(
            aux::RTPSReader* reader,
            const aux::CacheChange_t* const change) override;

private:

    //!Pointer to the EDPServer
    EDPServer2* sedp_;
};

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* EDPSERVERLISTENER2_H_ */
