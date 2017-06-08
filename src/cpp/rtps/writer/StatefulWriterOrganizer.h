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
 * @file StatefulWriterOrganizer.h
 */

#ifndef _RTPS_WRITER_STATEFULWRITERORGANIZER_H_
#define _RTPS_WRITER_STATEFULWRITERORGANIZER_H_

#include <vector>
#include <set>
#include <assert.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class StatefulWriterOrganizer
{
    public:
        
        StatefulWriterOrganizer() : mLastReader_(nullptr) {}

        void add_sequence_number(const SequenceNumber_t& seqNum, ReaderProxy* remoteReader)
        {
            if(mLastGuid_ != remoteReader->m_att.guid)
                organize();

            mLastGuid_ = remoteReader->m_att.guid;
            mLastReader_ = remoteReader;
            mLastSeqList_.insert(seqNum);
        }

        void organize()
        {
            if(mLastGuid_ != GUID_t::unknown())
            {
                assert(mLastReader_ != nullptr);

                bool inserted = false;

                for(auto pair : mElements_)
                {
                    if(pair.second == mLastSeqList_)
                    {
                        pair.first.push_back(mLastReader_);
                        mLastSeqList_.clear();
                        inserted = true;
                        break;
                    }
                }

                if(!inserted)
                    mElements_.push_back(std::pair<std::vector<ReaderProxy*>, std::set<SequenceNumber_t>>({mLastReader_}, std::move(mLastSeqList_)));

                mLastGuid_ = GUID_t::unknown();
                mLastReader_ = nullptr;
            }
        }

    private:

        GUID_t mLastGuid_;

        ReaderProxy* mLastReader_;

        std::set<SequenceNumber_t> mLastSeqList_;

        std::vector<std::pair<std::vector<ReaderProxy*>, std::set<SequenceNumber_t>>> mElements_;

    public:

        decltype(mElements_)& elements() { organize(); return mElements_; }

};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_WRITER_STATEFULWRITERORGANIZER_H_
