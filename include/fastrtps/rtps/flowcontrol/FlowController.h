#ifndef FLOW_CONTROLLER_H
#define FLOW_CONTROLLER_H

#include <vector>
#include <mutex>
#include <functional>
#include <memory>
#include <fastrtps/rtps/common/CacheChange.h>
#include <fastrtps/rtps/messages/RTPSMessageGroup.h>

// Boost forward declarations
namespace boost{ class thread; namespace asio{ class io_service; }}
namespace eprosima{
namespace fastrtps{
namespace rtps{

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
   static void NotifyControllersChangeSent(const CacheChangeForGroup_t*);

   //! Controller operator. Transforms the vector of changes in place.
   virtual void operator()(std::vector<CacheChangeForGroup_t>& changes) = 0;

   virtual ~FlowController();
   FlowController();

private:
   virtual void NotifyChangeSent(const CacheChangeForGroup_t*){};
   void RegisterAsListeningController();
   void DeRegisterAsListeningController();

   static std::vector<FlowController*> ListeningControllers;
   static std::unique_ptr<boost::thread> ControllerThread;
   static void StartControllerService();
   static void StopControllerService();

   // No copy, assignment or move! Controllers are accessed by reference
   // from several places.
   // Ownership to be transferred via unique_ptr move semantics only.
   const FlowController& operator=(const FlowController&) = delete;
   FlowController(const FlowController&) = delete;
   FlowController(FlowController&&) = delete;

protected:
   static std::recursive_mutex FlowControllerMutex;
	static std::unique_ptr<boost::asio::io_service> ControllerService;

public:
   // To be used by derived filters to schedule asynchronous operations.
   static bool IsListening(FlowController*);
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
