#include <fastrtps/rtps/resources/AsyncInterestTree.h>
#include "../participant/RTPSParticipantImpl.h"

AsyncInterestTree::AsyncInterestTree():
   mActiveInterest(&mInterestAlpha),
   mHiddenInterest(&mInterestBeta)
{
}

void AsyncInterestTree::RegisterInterest(const RTPSWriter* writer)
{
   std::unique_lock<std::mutex> guard(mMutexHidden);
   mHiddenInterest->insert(writer); 
}

void AsyncInterestTree::RegisterInterest(const RTPSParticipantImpl* participant)
{
   std::unique_lock<std::mutex> guard(mMutexHidden);
   auto writers = participant->getAllWriters();

   for (auto writer : writers)
      mHiddenInterest->insert(writer); 
}

void AsyncInterestTree::Swap()
{
   std::unique_lock<std::mutex> activeGuard(mMutexActive);
   std::unique_lock<std::mutex> hiddenGuard(mMutexHidden);

   mActiveInterest->clear();
   auto swap = mActiveInterest;
   mActiveInterest = mHiddenInterest;
   mHiddenInterest = swap;
}

std::set<const RTPSWriter*> AsyncInterestTree::GetInterestedWriters() const
{
   std::unique_lock<std::mutex> activeGuard(mMutexActive);
   return *mActiveInterest;
}
