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
 * @file TypeLookupReplyListener.hpp
 *
 */

#ifndef _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REPLY_LISTENER_HPP_
#define _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REPLY_LISTENER_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSReader;
struct CacheChange_t;

} // namespace rtps

namespace dds {
namespace builtin {

class TypeLookupManager;

struct ReplyWithServerGUID
{
    TypeLookup_Reply reply;
    fastdds::rtps::GUID_t type_server;
};

/**
 * Class TypeLookupReplyListener that receives the typelookup request messages of remote endpoints.
 * @ingroup TYPES_MODULE
 */
class TypeLookupReplyListener : public fastdds::rtps::ReaderListener, public fastdds::rtps::WriterListener
{
public:

    /**
     * @brief Constructor
     * @param manager Pointer to the TypeLookupManager.
     */
    TypeLookupReplyListener(
            TypeLookupManager* manager);

    /**
     * @brief Destructor
     */
    virtual ~TypeLookupReplyListener() override;

protected:

    /**
     * @brief Starts the thread that process the received replies.
     */
    void start_reply_processor_thread();

    /**
     * @brief Stops the thread that process the received replies.
     */
    void stop_reply_processor_thread();

    /**
     * @brief Process the replies in the queue.
     */
    void process_reply();

    /**
     * @brief Registers TypeIdentifier and TypeObject in TypeObjectRegistry.
     * This method also notifies all type related callbacks and removes the current SampleIdentity from the pending request list.
     * @param request_id[in] The SampleIdentity of the request.
     * @param reply[in] The reply data.
     * @param related_request[in] The request that this reply answers.
     */
    void check_get_types_reply(
            const SampleIdentity& request_id,
            const xtypes::TypeIdentfierWithSize& type_id,
            const TypeLookup_getTypes_Out& reply,
            SampleIdentity related_request);

    /**
     * @brief Checks if all dependencies are solved.
     * If they are not, sends next request and adds it to the list.
     * If they are, sends get_types request and adds it to the list.
     * Also removes the current SampleIdentity from the list.
     * @param request_id[in] The SampleIdentity of the request.
     * @param type_server[in] GUID corresponding to the remote participant which TypeInformation is being solved.
     * @param reply[in] The reply data.
     */
    void check_get_type_dependencies_reply(
            const SampleIdentity& request_id,
            const xtypes::TypeIdentfierWithSize& type_id,
            const fastdds::rtps::GUID_t type_server,
            const TypeLookup_getTypeDependencies_Out& reply);

    /**
     * @brief Method called when this class is notified of a new cache change.
     * @param reader The reader receiving the cache change.
     * @param change The cache change.
     */
    void on_new_cache_change_added(
            fastdds::rtps::RTPSReader* reader,
            const fastdds::rtps::CacheChange_t* const change) override;

    void on_writer_change_received_by_all(
            fastdds::rtps::RTPSWriter*,
            fastdds::rtps::CacheChange_t* change) override;

    //! A pointer to the typelookup manager.
    TypeLookupManager* typelookup_manager_;

    //! Mutex to protect access to replies_with_continuation_.
    std::mutex replies_with_continuation_mutex_;

    //! Collection of the replies that needed continuation points.
    std::vector<SampleIdentity> replies_with_continuation_;

    eprosima::thread replies_processor_thread;
    std::queue<ReplyWithServerGUID> replies_queue_;
    std::mutex replies_processor_cv_mutex_;
    std::condition_variable replies_processor_cv_;
    bool processing_ = false;
    rtps::ThreadSettings replies_processor_thread_settings_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REPLY_LISTENER_HPP_*/
