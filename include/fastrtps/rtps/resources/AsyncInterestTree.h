#ifndef ASYNC_INTEREST_TREE_H
#define ASYNC_INTEREST_TREE_H

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <mutex>
#include <set>

namespace eprosima {
namespace fastrtps{
namespace rtps {

class AsyncInterestTree
{
public:

   AsyncInterestTree();
   /* *
    * Registers a writer in a hidden set.
    * Threadsafe thanks to set swap.
    */
   void RegisterInterest(const RTPSWriter*);

   /* *
    * Registers all writers from  participant in a hidden set.
    * Threadsafe thanks to set swap.
    */
   void RegisterInterest(const RTPSParticipantImpl*);

   /* *
    * Clears the visible set and swaps
    * with the hidden set.
    */
   void Swap();

   //! Extracts from the visible set 
   std::set<const RTPSWriter*> GetInterestedWriters() const;

private:
   std::set<const RTPSWriter*> mInterestAlpha, mInterestBeta;
   mutable std::mutex mMutexActive, mMutexHidden;
   
   std::set<const RTPSWriter*>* mActiveInterest;
   std::set<const RTPSWriter*>* mHiddenInterest;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
