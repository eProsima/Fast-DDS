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

#include <fastrtps/rtps/flowcontrol/FlowController.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using namespace eprosima::fastrtps::rtps;

std::vector<FlowController*> FlowController::ListeningControllers;
std::recursive_mutex FlowController::FlowControllerMutex;
std::unique_ptr<boost::thread> FlowController::ControllerThread;
std::unique_ptr<boost::asio::io_service> FlowController::ControllerService;

FlowController::FlowController()
{
   if (!ControllerService)
      ControllerService.reset(new boost::asio::io_service);
   RegisterAsListeningController();
}

FlowController::~FlowController()
{
   DeRegisterAsListeningController();
}

void FlowController::NotifyControllersChangeSent(CacheChange_t* change)
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowControllerMutex);
   for (auto filter : ListeningControllers)
      filter->NotifyChangeSent(change);
}

void FlowController::RegisterAsListeningController()
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowControllerMutex);
   ListeningControllers.push_back(this);

   if (!ControllerThread)
      StartControllerService();
}

void FlowController::DeRegisterAsListeningController()
{
    FlowControllerMutex.lock();
    ListeningControllers.erase(std::remove(ListeningControllers.begin(), ListeningControllers.end(), this), ListeningControllers.end());
    FlowControllerMutex.unlock();
    if (ListeningControllers.empty() && ControllerThread)
        StopControllerService();
}

bool FlowController::IsListening(FlowController* filter) 
{
   std::unique_lock<std::recursive_mutex> scopedLock(FlowControllerMutex);
   auto it = find(ListeningControllers.begin(), ListeningControllers.end(), filter);
   return it != ListeningControllers.end();
}

void FlowController::StartControllerService()
{
   auto ioServiceFunction = [&]()
   {
      ControllerService->reset();
      boost::asio::io_service::work work(*ControllerService);
      ControllerService->run();
   };
   ControllerThread.reset(new boost::thread(ioServiceFunction));
}

void FlowController::StopControllerService()
{
   ControllerService->stop();
   ControllerThread->join();
   ControllerThread.reset();
}
