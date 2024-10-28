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

#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using EndpointAddedCallback = std::function<
    void (RTPSReader* reader, const CacheChange_t* change)>;

class RTPSReader;
struct CacheChange_t;

/*!
 * Placeholder class EDPListener
 * @ingroup DISCOVERY_MODULE
 */

class EDPListener : public ReaderListener, public WriterListener
{
public:

    /**
     * @param change
     */
    bool computeKey(
            CacheChange_t* change);

protected:

    //! returns true if loading info from persistency database
    static bool ongoingDeserialization(
            EDP* edp);
};

/**
 * Placeholder class used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPBasePUBListener : public EDPListener
{
public:

    EDPBasePUBListener() = default;

    virtual ~EDPBasePUBListener() = default;

protected:

    void add_writer_from_change(
            RTPSReader* reader,
            ReaderHistory* reader_history,
            CacheChange_t* change,
            EDP* edp,
            bool release_change = true,
            const EndpointAddedCallback& writer_added_callback = nullptr
            );
};

/**
 * Placeholder class used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPBaseSUBListener : public EDPListener
{
public:

    EDPBaseSUBListener() = default;

    virtual ~EDPBaseSUBListener() = default;

protected:

    void add_reader_from_change(
            RTPSReader* reader,
            ReaderHistory* reader_history,
            CacheChange_t* change,
            EDP* edp,
            bool release_change = true,
            const EndpointAddedCallback& reader_added_callback = nullptr
            );
};

/*!
 * Class EDPSimplePUBReaderListener, used to define the behavior when a new WriterProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPSimplePUBListener : public EDPBasePUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPSimple associated with this listener.
     */
    EDPSimplePUBListener(
            EDPSimple* sedp)
        : sedp_(sedp)
    {
    }

    virtual ~EDPSimplePUBListener() = default;

    /**
     * Virtual method,
     * @param reader
     * @param change
     */
    void on_new_cache_change_added(
            RTPSReader* reader,
            const CacheChange_t* const change) override;


    /*!
     * This method is called when all the readers matched with this Writer acknowledge that a cache
     * change has been received.
     * @param writer Pointer to the RTPSWriter.
     * @param change Pointer to the affected CacheChange_t.
     */
    void on_writer_change_received_by_all(
            RTPSWriter* writer,
            CacheChange_t* change) override;

protected:

    //!Pointer to the EDPSimple
    EDPSimple* sedp_;
};

/*!
 * Class EDPSimpleSUBReaderListener, used to define the behavior when a new ReaderProxyData is received.
 * @ingroup DISCOVERY_MODULE
 */
class EDPSimpleSUBListener : public EDPBaseSUBListener
{
public:

    /*!
       Constructor
     * @param sedp Pointer to the EDPSimple associated with this listener.
     */
    EDPSimpleSUBListener(
            EDPSimple* sedp)
        : sedp_(sedp)
    {
    }

    virtual ~EDPSimpleSUBListener() = default;

    /**
     * @param reader
     * @param change
     */
    void on_new_cache_change_added(
            RTPSReader* reader,
            const CacheChange_t* const change) override;

    /*!
     * This method is called when all the readers matched with this Writer acknowledge that a cache
     * change has been received.
     * @param writer Pointer to the RTPSWriter.
     * @param change Pointer to the affected CacheChange_t.
     */
    void on_writer_change_received_by_all(
            RTPSWriter* writer,
            CacheChange_t* change) override;

private:

    //!Pointer to the EDPSimple
    EDPSimple* sedp_;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* EDPSIMPLELISTENER_H_ */
