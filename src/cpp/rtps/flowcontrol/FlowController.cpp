#include <fastrtps/rtps/flowcontrol/FlowController.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

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

void FlowController::NotifyControllersChangeSent(const CacheChangeForGroup_t* change)
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
   std::unique_lock<std::recursive_mutex> scopedLock(FlowControllerMutex);
   ListeningControllers.erase(std::remove(ListeningControllers.begin(), ListeningControllers.end(), this), ListeningControllers.end());
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
