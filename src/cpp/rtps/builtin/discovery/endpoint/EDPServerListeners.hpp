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
namespace fastdds {
namespace rtps {

class RTPSReader;
struct CacheChange_t;
class BaseReader;
class PDPServer;
class EDPServer;

/*!
 * Class EDPServerPUBListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerPUBListener : public fastdds::rtps::EDPBasePUBListener
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
    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader,
            const fastdds::rtps::CacheChange_t* const change) override;

private:

    std::string get_writer_proxy_topic_name(
            fastdds::rtps::GUID_t auxGUID);

    void notify_discoverydatabase(
            std::string topic_name,
            BaseReader* reader,
            fastdds::rtps::CacheChange_t* change);

    void continue_with_writer(
            BaseReader* reader,
            fastdds::rtps::CacheChange_t* change);

    //!Pointer to the EDPServer
    EDPServer* sedp_;
};

/*!
 * Class EDPServerSUBListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPServerSUBListener : public fastdds::rtps::EDPBaseSUBListener
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
    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader,
            const fastdds::rtps::CacheChange_t* const change) override;

private:

    std::string get_reader_proxy_topic_name(
            fastdds::rtps::GUID_t auxGUID);

    void notify_discoverydatabase(
            std::string topic_name,
            BaseReader* reader,
            fastdds::rtps::CacheChange_t* change);

    void continue_with_reader(
            BaseReader* reader,
            fastdds::rtps::CacheChange_t* change);

    //!Pointer to the EDPServer
    EDPServer* sedp_;
};

} /* namespace rtps */
} // namespace fastdds
} /* namespace eprosima */
#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* EDPSERVERLISTENER2_H_ */
