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
 * @file TypeLookupRequestListener.hpp
 *
 */

#ifndef _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REQUEST_LISTENER_HPP_
#define _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REQUEST_LISTENER_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <utility>

#include <fastdds/builtin/type_lookup_service/detail/TypeLookupTypes.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>
#include <fastdds/xtypes/type_representation/TypeIdentifierWithSizeHashSpecialization.h>

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

/**
 * Class TypeLookupRequestListener that receives the typelookup request messages of remote endpoints.
 * @ingroup TYPES_MODULE
 */
class TypeLookupRequestListener : public fastdds::rtps::ReaderListener, public fastdds::rtps::WriterListener
{
    friend class TypeLookupManager;

public:

    /**
     * @brief Constructor.
     * @param manager Pointer to the TypeLookupManager.
     */
    TypeLookupRequestListener(
            TypeLookupManager* manager);

    /**
     * @brief Destructor.
     */
    virtual ~TypeLookupRequestListener() override;

protected:

    /**
     * @brief Starts the thread that process the received requests.
     */
    void start_request_processor_thread();

    /**
     * @brief Stops the thread that process the received requests.
     */
    void stop_request_processor_thread();

    /**
     * @brief Process the requests in the queue.
     */
    void process_requests();

    /**
     * @brief Gets TypeObject from TypeObjectRegistry, creates and sends reply.
     * @param request_id[in] The SampleIdentity of the request.
     * @param request[in] The request data.
     * @param vendor_id[in] Vendor identifier that sent the request.
     */
    void check_get_types_request(
            SampleIdentity request_id,
            const TypeLookup_getTypes_In& request,
            const rtps::VendorId_t& vendor_id);

    /**
     * @brief Gets type dependencies from TypeObjectRegistry, creates and sends reply.
     * @param request_id[in] The SampleIdentity of the request.
     * @param request[in] The request data.
     */
    void check_get_type_dependencies_request(
            SampleIdentity request_id,
            const TypeLookup_getTypeDependencies_In& request);

    /**
     * @brief Creates a TypeLookup_getTypeDependencies_Out using continuation points to manage size.
     * @param type_dependencies[in] The full list of dependencies of the type.
     * @param continuation_point[in] The continuation point of the previous request.
     * @param out[out] The reply containing the dependent types.
     * @return false if the continuation point is invalid, true otherwise.
     */
    bool prepare_get_type_dependencies_response(
            const std::unordered_set<xtypes::TypeIdentfierWithSize>& type_dependencies,
            const std::vector<uint8_t>& continuation_point,
            TypeLookup_getTypeDependencies_Out& out) const;

    /**
     * @brief Creates and sends the TypeLookup_Reply.
     * @param request_id[in] The SampleIdentity of the request.
     * @param exception_code[in] The RemoteExceptionCode_t used in the header.
     * @param out[in] Reply to the query included in the received request.
     */
    void answer_request(
            SampleIdentity request_id,
            rpc::RemoteExceptionCode_t exception_code,
            TypeLookup_getTypeDependencies_Out& out);
    void
    answer_request(
            SampleIdentity request_id,
            rpc::RemoteExceptionCode_t exception_code,
            TypeLookup_getTypes_Out& out);
    void answer_request(
            SampleIdentity request_id,
            rpc::RemoteExceptionCode_t exception_code);

    /**
     * @brief Method call when this class is notified of a new cache change.
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

    eprosima::thread request_processor_thread;
    std::queue<std::pair<TypeLookup_Request, rtps::VendorId_t>> requests_queue_;
    std::mutex request_processor_cv_mutex_;
    std::condition_variable request_processor_cv_;
    bool processing_ = false;
    rtps::ThreadSettings request_processor_thread_settings_;
};

} /* namespace builtin */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
#endif /* _FASTDDS_BUILTIN_TYPE_LOOKUP_SERVICE_TYPE_LOOKUP_REQUEST_LISTENER_HPP_*/
