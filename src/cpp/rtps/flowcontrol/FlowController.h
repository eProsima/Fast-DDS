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

#ifndef FLOW_CONTROLLER_H
#define FLOW_CONTROLLER_H

#include <fastdds/rtps/common/CacheChange.h>
#include <rtps/writer/RTPSWriterCollector.h>

#include <vector>
#include <mutex>
#include <functional>
#include <memory>
#include <thread>

namespace eprosima{
namespace fastrtps{
namespace rtps{

class ReaderLocator;
class ReaderProxy;

/**
 * Flow Controllers take a vector of cache changes (by reference) and return a filtered
 * vector, with a collection of changes this filter considers valid for sending,
 * ordered by its subjective priority.
 * @ingroup NETWORK_MODULE.
 * */
class FlowController
{
    public:
        //! Called when a change is finally dispatched.
        static void NotifyControllersChangeSent(CacheChange_t*);

        //! Controller operator. Transforms the vector of changes in place.
        virtual void operator()(RTPSWriterCollector<ReaderLocator*>& changesToSend) = 0;
        virtual void operator()(RTPSWriterCollector<ReaderProxy*>& changesToSend) = 0;

        virtual void disable() = 0;

        virtual ~FlowController();
        FlowController();

    private:
        virtual void NotifyChangeSent(CacheChange_t*){};
        void RegisterAsListeningController();
        void DeRegisterAsListeningController();

        static std::vector<FlowController*> ListeningControllers;

        // No copy, assignment or move! Controllers are accessed by reference
        // from several places.
        // Ownership to be transferred via unique_ptr move semantics only.
        const FlowController& operator=(const FlowController&) = delete;
        FlowController(const FlowController&) = delete;
        FlowController(FlowController&&) = delete;

    protected:
        static std::recursive_mutex FlowControllerMutex;

    public:
        // To be used by derived filters to schedule asynchronous operations.
        static bool IsListening(FlowController*);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
