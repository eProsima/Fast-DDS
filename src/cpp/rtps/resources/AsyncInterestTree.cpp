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

#include <mutex>

#include <fastrtps/rtps/resources/AsyncInterestTree.h>
#include <rtps/participant/RTPSParticipantImpl.h>

using namespace eprosima::fastrtps::rtps;

AsyncInterestTree::AsyncInterestTree():
    mActiveInterest(&mInterestAlpha),
    mHiddenInterest(&mInterestBeta)
{
}

void AsyncInterestTree::RegisterInterest(
        const RTPSWriter* writer)
{
    std::unique_lock<std::timed_mutex> guard(mMutexHidden);
    mHiddenInterest->insert(writer); 
}

bool AsyncInterestTree::RegisterInterest(
        const RTPSWriter* writer,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool ret_value = false;
    std::unique_lock<std::timed_mutex> guard(mMutexHidden, std::defer_lock);

    if(guard.try_lock_until(max_blocking_time))
    {
        mHiddenInterest->insert(writer); 
        ret_value = true;
    }

    return ret_value;
}

void AsyncInterestTree::Swap()
{
    std::unique_lock<std::timed_mutex> activeGuard(mMutexActive);
    std::unique_lock<std::timed_mutex> hiddenGuard(mMutexHidden);

    mActiveInterest->clear();
    auto swap = mActiveInterest;
    mActiveInterest = mHiddenInterest;
    mHiddenInterest = swap;
}

std::set<const RTPSWriter*> AsyncInterestTree::GetInterestedWriters() const
{
    std::unique_lock<std::timed_mutex> activeGuard(mMutexActive);
    return *mActiveInterest;
}
