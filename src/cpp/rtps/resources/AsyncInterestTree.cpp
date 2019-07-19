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

#include <fastdds/rtps/resources/AsyncInterestTree.h>
#include <rtps/participant/RTPSParticipantImpl.h>

using namespace eprosima::fastrtps::rtps;

bool AsyncInterestTree::register_interest(
        RTPSWriter* writer)
{
    std::unique_lock<std::timed_mutex> guard(mMutexHidden);
    return register_interest_nts(writer);
}

bool AsyncInterestTree::register_interest(
        RTPSWriter* writer,
        const std::chrono::time_point<std::chrono::steady_clock>& max_blocking_time)
{
    bool ret_value = false;
    std::unique_lock<std::timed_mutex> guard(mMutexHidden, std::defer_lock);

    if(guard.try_lock_until(max_blocking_time))
    {
        ret_value = register_interest_nts(writer);
    }

    return ret_value;
}

bool AsyncInterestTree::register_interest_nts(
        RTPSWriter* writer)
{
    RTPSWriter *curr = hidden_front_, *prev = nullptr;

    while (curr)
    {
        if (writer == curr)
        {
            return false;
        }

        prev = curr;
        curr = curr->next_[hidden_pos_];
    }

    if (!prev)
    {
        hidden_front_ = writer;
    }
    else
    {
        prev->next_[hidden_pos_] = writer;
    }

    return true;
}

bool AsyncInterestTree::unregister_interest(
        RTPSWriter* writer)
{
    std::unique_lock<std::timed_mutex> activeGuard(mMutexActive);
    std::unique_lock<std::timed_mutex> hiddenGuard(mMutexHidden);

    RTPSWriter *curr = active_front_, *prev = nullptr;

    while (curr)
    {
        if (curr == writer)
        {
            if (prev)
            {
                prev->next_[active_pos_] = curr->next_[active_pos_];
            }
            else
            {
                active_front_ = curr->next_[active_pos_];
            }

            curr->next_[active_pos_] = nullptr;
            break;
        }
        else
        {
            prev = curr;
            curr = curr->next_[active_pos_];
        }
    }

    curr = hidden_front_;
    prev = nullptr;

    while (curr)
    {
        if (curr == writer)
        {
            if (prev)
            {
                prev->next_[hidden_pos_] = curr->next_[hidden_pos_];
            }
            else
            {
                hidden_front_ = curr->next_[hidden_pos_];
            }

            curr->next_[hidden_pos_] = nullptr;
            break;
        }
        else
        {
            prev = curr;
            curr = curr->next_[hidden_pos_];
        }
    }

    return (active_front_ == nullptr && hidden_front_ == nullptr);
}

void AsyncInterestTree::swap()
{
    std::unique_lock<std::timed_mutex> activeGuard(mMutexActive);
    std::unique_lock<std::timed_mutex> hiddenGuard(mMutexHidden);

    active_front_ = hidden_front_;
    hidden_front_ = nullptr;
    active_pos_ = (active_pos_ + 1) & 0x1;
    hidden_pos_ = (hidden_pos_ + 1) & 0x1;
}

RTPSWriter* AsyncInterestTree::next_active_nts()
{
    RTPSWriter* ret_writer = active_front_;
    if (active_front_)
    {
        active_front_ = active_front_->next_[active_pos_];
        ret_writer->next_[active_pos_] = nullptr;
    }
    return ret_writer;
}
