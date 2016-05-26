#ifndef ASYNC_INTEREST_TREE_H
#define ASYNC_INTEREST_TREE_H

#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <mutex>

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
   void RegisterInterest(RTPSWriter*);

   /* *
    * Registers all writers from  participant in a hidden set.
    * Threadsafe thanks to set swap.
    */
   void RegisterInterest(RTPSParticipantImpl*);

   /* *
    * Clears the visible set and swaps
    * with the hidden structure.
    */
   void Swap();

   //! Extracts from the visible structure 
   std::set<RTPSWriter*> GetInterestedWriters() const;

private:
   std::set<RTPSWriter*> mInterestAlpha, mInterestBeta;
   mutable std::mutex mMutexActive, mMutexHidden;
   
   std::set<RTPSWriter*>* mActiveInterest;
   std::set<RTPSWriter*>* mHiddenInterest;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
