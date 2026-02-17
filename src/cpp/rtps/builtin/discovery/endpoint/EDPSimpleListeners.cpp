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
 * @file EDPSimpleListener.cpp
 *
 */

#include <rtps/builtin/discovery/endpoint/EDPSimpleListeners.h>

#include <mutex>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/network/NetworkFactory.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/writer/StatefulWriter.hpp>

using ParameterList = eprosima::fastdds::dds::ParameterList;


// Release reader lock to avoid ABBA lock. PDP mutex should always be first.
// Keep change information on local variables to check consistency later
#define PREVENT_PDP_DEADLOCK(reader, change, pdp)                         \
    GUID_t writer_guid = (change)->writerGUID;                            \
    SequenceNumber_t seq_num = (change)->sequenceNumber;                  \
    (reader)->getMutex().unlock();                                        \
    std::unique_lock<std::recursive_mutex> lock(*((pdp)->getMutex()));    \
    (reader)->getMutex().lock();                                          \
                                                                          \
    if ((ALIVE != (change)->kind) ||                                      \
            (seq_num != (change)->sequenceNumber) ||                      \
            (writer_guid != (change)->writerGUID))                        \
    {                                                                     \
        return;                                                           \
    }                                                                     \
    (void)seq_num

namespace eprosima {
namespace fastdds {
namespace rtps {

void EDPBasePUBListener::add_writer_from_change(
        RTPSReader* reader,
        ReaderHistory* reader_history,
        CacheChange_t* change,
        EDP* edp,
        bool release_change /*= true*/,
        const EndpointAddedCallback& writer_added_callback /* = nullptr*/)
{
    //LOAD INFORMATION IN DESTINATION WRITER PROXY DATA
    NetworkFactory& network = edp->mp_RTPSParticipant->network_factory();
    CDRMessage_t tempMsg(change->serializedPayload);
    auto temp_writer_data = edp->get_temporary_writer_proxies_pool().get();
    const auto type_server = change->writerGUID;

    if (temp_writer_data->read_from_cdr_message(&tempMsg, change->vendor_id))
    {
        if (temp_writer_data->guid.guidPrefix == edp->mp_RTPSParticipant->getGuid().guidPrefix)
        {
            EPROSIMA_LOG_INFO(RTPS_EDP, "Message from own RTPSParticipant, ignoring");
            return;
        }

        // Callback function to continue after typelookup is complete
        fastdds::dds::builtin::AsyncGetTypeWriterCallback after_typelookup_callback =
                [reader, change, edp, &network, writer_added_callback]
                    (eprosima::fastdds::dds::ReturnCode_t request_ret_status,
                        eprosima::fastdds::rtps::WriterProxyData* temp_writer_data)
                {
                    //LOAD INFORMATION IN DESTINATION WRITER PROXY DATA
                    auto copy_data_fun = [&request_ret_status, &temp_writer_data, &network](
                        WriterProxyData* data,
                        bool updating,
                        const ParticipantProxyData& participant_data)
                            {
                                if (updating && !data->is_update_allowed(*temp_writer_data))
                                {
                                    EPROSIMA_LOG_WARNING(RTPS_EDP,
                                            "Received incompatible update for WriterQos. writer_guid = " <<
                                            data->guid);
                                }
                                *data = *temp_writer_data;
                                data->setup_locators(*temp_writer_data, network, participant_data);

                                if (request_ret_status != fastdds::dds::RETCODE_OK)
                                {
                                    data->type_information.clear();
                                }
                                return true;
                            };

                    GUID_t participant_guid;
                    WriterProxyData* writer_data =
                            edp->mp_PDP->addWriterProxyData(temp_writer_data->guid, participant_guid, copy_data_fun);

                    if (writer_data != nullptr)
                    {
                        edp->pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
                        if (nullptr != writer_added_callback)
                        {
                            writer_added_callback(reader, change);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_WARNING(RTPS_EDP, "Received message from UNKNOWN RTPSParticipant, removing");
                    }
                };

        // Remove change from history.
        reader_history->remove_change(reader_history->find_change(change), release_change);

        // At this point, we can release the reader lock because the change is not used
        reader->getMutex().unlock();

        auto typelookup_manager = edp->mp_RTPSParticipant->typelookup_manager();

        // Check if TypeInformation exists to start the typelookup service
        if (nullptr != typelookup_manager && temp_writer_data->type_information.assigned())
        {
            typelookup_manager->async_get_type(
                temp_writer_data,
                type_server,
                after_typelookup_callback);
        }
        // If TypeInformation does not exist, try fallback mechanism
        else
        {
            EPROSIMA_LOG_INFO(
                RTPS_EDP, "EDPSimpleListener: No TypeLookupManager or TypeInformation. Trying fallback mechanism");
            after_typelookup_callback(fastdds::dds::RETCODE_NO_DATA, temp_writer_data.get());
        }
        // Release temporary proxy
        temp_writer_data.reset();


        // Take the reader lock again if needed.
        reader->getMutex().lock();
    }
}

void EDPSimplePUBListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->publications_reader_.first->getMutex());
    EPROSIMA_LOG_INFO(RTPS_EDP, "");
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Received change with no Key");
    }

    ReaderHistory* reader_history =
#if HAVE_SECURITY
            reader == sedp_->publications_secure_reader_.first ?
            sedp_->publications_secure_reader_.second :
#endif // if HAVE_SECURITY
            sedp_->publications_reader_.second;

    if (change->kind == ALIVE)
    {
        PREVENT_PDP_DEADLOCK(reader, change, sedp_->mp_PDP);

        // Note: change is removed from history inside this method.
        add_writer_from_change(reader, reader_history, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        EPROSIMA_LOG_INFO(RTPS_EDP, "Disposed Remote Writer, removing...");
        GUID_t writer_guid = iHandle2GUID(change->instanceHandle);
        //Removing change from history
        reader_history->remove_change(change);
        reader->getMutex().unlock();
        this->sedp_->mp_PDP->removeWriterProxyData(writer_guid);
        reader->getMutex().lock();
    }
}

bool EDPListener::computeKey(
        CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, fastdds::dds::PID_ENDPOINT_GUID);
}

void EDPBaseSUBListener::add_reader_from_change(
        RTPSReader* reader,
        ReaderHistory* reader_history,
        CacheChange_t* change,
        EDP* edp,
        bool release_change /*= true*/,
        const EndpointAddedCallback& reader_added_callback /* = nullptr*/)
{
    //LOAD INFORMATION IN TEMPORAL READER PROXY DATA
    NetworkFactory& network = edp->mp_RTPSParticipant->network_factory();
    CDRMessage_t tempMsg(change->serializedPayload);
    auto temp_reader_data = edp->get_temporary_reader_proxies_pool().get();
    const auto type_server = change->writerGUID;

    if (temp_reader_data->read_from_cdr_message(&tempMsg, change->vendor_id))
    {
        if (temp_reader_data->guid.guidPrefix == edp->mp_RTPSParticipant->getGuid().guidPrefix)
        {
            EPROSIMA_LOG_INFO(RTPS_EDP, "From own RTPSParticipant, ignoring");
            return;
        }

        // Callback function to continue after typelookup is complete
        fastdds::dds::builtin::AsyncGetTypeReaderCallback after_typelookup_callback =
                [reader, change, edp, &network, reader_added_callback]
                    (eprosima::fastdds::dds::ReturnCode_t request_ret_status,
                        eprosima::fastdds::rtps::ReaderProxyData* temp_reader_data)
                {
                    //LOAD INFORMATION IN DESTINATION READER PROXY DATA
                    auto copy_data_fun = [&request_ret_status, &temp_reader_data, &network](
                        ReaderProxyData* data,
                        bool updating,
                        const ParticipantProxyData& participant_data)
                            {
                                if (updating && !data->is_update_allowed(*temp_reader_data))
                                {
                                    EPROSIMA_LOG_WARNING(RTPS_EDP,
                                            "Received incompatible update for ReaderQos. reader_guid = " <<
                                            data->guid);
                                }
                                *data = *temp_reader_data;
                                data->setup_locators(*temp_reader_data, network, participant_data);

                                if (request_ret_status != fastdds::dds::RETCODE_OK)
                                {
                                    data->type_information.clear();
                                }
                                return true;
                            };

                    //LOOK IF IS AN UPDATED INFORMATION
                    GUID_t participant_guid;
                    ReaderProxyData* reader_data =
                            edp->mp_PDP->addReaderProxyData(temp_reader_data->guid, participant_guid, copy_data_fun);

                    if (reader_data != nullptr) //ADDED NEW DATA
                    {
                        edp->pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
                        if (nullptr != reader_added_callback)
                        {
                            reader_added_callback(reader, change);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_WARNING(RTPS_EDP, "From UNKNOWN RTPSParticipant, removing");
                    }
                };

        // Remove change from history.
        reader_history->remove_change(reader_history->find_change(change), release_change);

        // At this point, we can release the reader lock because the change is not used
        reader->getMutex().unlock();

        auto typelookup_manager = edp->mp_RTPSParticipant->typelookup_manager();

        // Check if TypeInformation exists to start the typelookup service
        if (nullptr != typelookup_manager && temp_reader_data->type_information.assigned())
        {
            typelookup_manager->async_get_type(
                temp_reader_data,
                type_server,
                after_typelookup_callback);
        }
        // If TypeInformation does not exist, try fallback mechanism
        else
        {
            EPROSIMA_LOG_INFO(
                RTPS_EDP, "EDPSimpleListener: No TypeLookupManager or TypeInformation. Trying fallback mechanism");
            after_typelookup_callback(fastdds::dds::RETCODE_NO_DATA, temp_reader_data.get());
        }
        // Release the temporary proxy
        temp_reader_data.reset();

        // Take the reader lock again if needed.
        reader->getMutex().lock();
    }
}

void EDPSimpleSUBListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->subscriptions_reader_.first->getMutex());
    EPROSIMA_LOG_INFO(RTPS_EDP, "");
    if (!computeKey(change))
    {
        EPROSIMA_LOG_WARNING(RTPS_EDP, "Received change with no Key");
    }

    ReaderHistory* reader_history =
#if HAVE_SECURITY
            reader == sedp_->subscriptions_secure_reader_.first ?
            sedp_->subscriptions_secure_reader_.second :
#endif // if HAVE_SECURITY
            sedp_->subscriptions_reader_.second;

    if (change->kind == ALIVE)
    {
        PREVENT_PDP_DEADLOCK(reader, change, sedp_->mp_PDP);

        // Note: change is removed from history inside this method.
        add_reader_from_change(reader, reader_history, change, sedp_);
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        EPROSIMA_LOG_INFO(RTPS_EDP, "Disposed Remote Reader, removing...");

        GUID_t reader_guid = iHandle2GUID(change->instanceHandle);
        //Removing change from history
        reader_history->remove_change(change);
        reader->getMutex().unlock();
        this->sedp_->mp_PDP->removeReaderProxyData(reader_guid);
        reader->getMutex().lock();
    }
}

void EDPSimplePUBListener::on_writer_change_received_by_all(
        RTPSWriter* writer,
        CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
                writer == sedp_->publications_secure_writer_.first ?
                sedp_->publications_secure_writer_.second :
#endif // if HAVE_SECURITY
                sedp_->publications_writer_.second;

        writer_history->remove_change(change);
    }
}

void EDPSimpleSUBListener::on_writer_change_received_by_all(
        RTPSWriter* writer,
        CacheChange_t* change)
{
    (void)writer;

    if (ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
                writer == sedp_->subscriptions_secure_writer_.first ?
                sedp_->subscriptions_secure_writer_.second :
#endif // if HAVE_SECURITY
                sedp_->subscriptions_writer_.second;

        writer_history->remove_change(change);
    }

}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
