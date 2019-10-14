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

/**
 * @file RTPSWriterCollector.h
 */

#ifndef _RTPS_WRITER_RTPSWRITERCOLLECTOR_H_
#define _RTPS_WRITER_RTPSWRITERCOLLECTOR_H_

#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/rtps/common/FragmentNumber.h>
#include <fastdds/rtps/common/CacheChange.h>

#include <vector>
#include <cassert>

namespace eprosima {
namespace fastrtps {
namespace rtps {

template<class T>
class RTPSWriterCollector
{
    public:

        struct Item
        {
            Item(SequenceNumber_t seqNum, FragmentNumber_t fragNum,
                    CacheChange_t* c) : sequenceNumber(seqNum),
                                        fragmentNumber(fragNum),
                                        cacheChange(c)

            {
                assert(seqNum == c->sequenceNumber);
            }
            //! Sequence number of the CacheChange.
            SequenceNumber_t sequenceNumber;
            /*!
             *  Fragment number of the represented fragment.
             *  If value is zero, it represents a whole change.
             */
            FragmentNumber_t fragmentNumber;

            CacheChange_t* cacheChange;

            mutable std::vector<T> remoteReaders;
        };

        struct ItemCmp
        {
            bool operator()(const Item& a, const Item& b) const
            {
                if(a.sequenceNumber < b.sequenceNumber)
                    return true;
                else if(a.sequenceNumber == b.sequenceNumber)
                    if(a.fragmentNumber < b.fragmentNumber)
                        return true;

                return false;
            }
        };

        typedef std::set<Item, ItemCmp> ItemSet;

        void add_change(CacheChange_t* change, const T& remoteReader, const FragmentNumberSet_t optionalFragmentsNotSent)
        {
            if(change->getFragmentSize() > 0)
            {
                optionalFragmentsNotSent.for_each([this, change, remoteReader](FragmentNumber_t sn)
                {
                    assert(sn <= change->getFragmentCount());
                    auto it = mItems_.emplace(change->sequenceNumber, sn, change);
                    it.first->remoteReaders.push_back(remoteReader);
                });
            }
            else
            {
                auto it = mItems_.emplace(change->sequenceNumber, 0, change);
                it.first->remoteReaders.push_back(remoteReader);
            }
        }

        bool empty()
        {
            return mItems_.empty();
        }

        size_t size()
        {
            return mItems_.size();
        }

        Item pop()
        {
            auto it = mItems_.begin();
            Item ret = *it;
            mItems_.erase(it);
            return ret;
        }

        void clear()
        {
            return mItems_.clear();
        }

        ItemSet& items()
        {
            return mItems_;
        }

    private:

        ItemSet mItems_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_WRITER_RTPSWRITERCOLLECTOR_H_
