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
 * @file FragmentedChangePitStop.h
 */
#ifndef _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_
#define _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/rtps/common/CacheChange.h>

#include <unordered_set>

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            // Forward definitions.
            class RTPSReader;

            /*!
             * @brief Manages not completed fragmented CacheChanges in reader side.
             * @remarks This class is non thread-safe.
             */
            class FragmentedChangePitStop
            {
                /*!
                 * @brief Objects used by FragmentedChangePitStop internally.
                 */
                class ChangeInPit
                {
                    public:

                        /*!
                         * @brief Relates a CacheChange_t with this ChangeInPit.
                         * This constructor has to be used if the ChangeInPit will be used to be stored in a container.
                         * @param change Related CacheChange_t.
                         */
                        ChangeInPit(CacheChange_t* change) : sequence_number_(change->sequenceNumber), change_(change) {};

                        /*!
                         * @brief Constructor used to generated a simple key for searching in a container.
                         * @param sequence_number SequenceNumber_t used as key.
                         * @remarks Not use this constructor if the object will be stored in a container.
                         */
                        ChangeInPit(const SequenceNumber_t &sequence_number) : sequence_number_(sequence_number), change_(nullptr) {};

                        ChangeInPit(const ChangeInPit& cip) : sequence_number_(cip.sequence_number_), change_(cip.change_) {};

                        CacheChange_t* getChange() const { return change_; }

                        bool operator==(const ChangeInPit& cip) const
                        {
                            return sequence_number_ == cip.sequence_number_;
                        }

                    private:

                        ChangeInPit& operator=(const ChangeInPit& cip) = delete;

                        const SequenceNumber_t sequence_number_;
                        CacheChange_t* change_;

                    public:
                        /*!
                         * @brief Defined the STD hash function for ChangeInPit.
                         */
                        struct ChangeInPitHash
                        {
                            std::size_t operator()(const ChangeInPit& cip) const
                            {
                                return SequenceNumberHash{}(cip.sequence_number_);
                            }
                        };
                };

                public:

                /*!
                 * @brief Default constructor.
                 * @param parent RTPSReader managing this object.
                 * It is necessary the access to reserve a new CacheChange_t.
                 */
                FragmentedChangePitStop(RTPSReader *parent) : parent_(parent) {}

                /*!
                 * @brief Process incomming fragments.
                 * @param incoming_change. CacheChange_t that stores the incomming fragments.
                 * @param sampleSize Total size of the sample. It was received in the DATA_FRAG submessage.
                 * @param fragmentStartingNum. First fragment number in the DATA_FRAG submessage.
                 * @return If a CacheChange_t is completed with new incomming fragments, this will be returned.
                 * In other case nullptr is returned.
                 */
                CacheChange_t* process(CacheChange_t* incoming_change, uint32_t sampleSize, uint32_t fragmentStartingNum);

                /*!
                 * @brief Search if there is a CacheChange_t, giving SequenceNumber_t and writer GUID_t,
                 * waiting to be completed because it is fragmented.
                 * @param sequence_number SequenceNumber_t of the searched CacheChange_t.
                 * @param writer_guid writer GUID_t of the searched CacheChange_t.
                 * @return If a CacheChange_t was found, it will be returned. In other case nullptr is returned.
                 */
                CacheChange_t* find(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid);

                /*!
                 * @brief Checks if there is a CacheChange_t, giving SequenceNumber_t and writer GUID_t.
                 * In case there is, it will be removed.
                 * @param sequence_number SequenceNumber_t of the searched CacheChange_t.
                 * @param writer_guid writer GUID_t of the searched CacheChange_t.
                 * @return If a CacheChange_t was found and removed, true value will be returned. In other case
                 * false value is returned.
                 */
                bool try_to_remove(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid);

                /*!
                 * @brief Checks if there are CacheChange_t, with writer GUID_t and a SequenceNumber_t lower than given SequenceNumber_t.
                 * In case there are, they will be removed.
                 * @param sequence_number SequenceNumber_t used as maximum.
                 * @param writer_guid writer GUID_t of the searched CacheChange_t.
                 * @return If some CacheChange_t were found and removed, true value will be returned. In other case
                 * false value is returned.
                 */
                bool try_to_remove_until(const SequenceNumber_t& sequence_number, const GUID_t& writer_guid);

                private:

                std::unordered_multiset<ChangeInPit, ChangeInPit::ChangeInPitHash> changes_;

                RTPSReader* parent_;

                FragmentedChangePitStop(const FragmentedChangePitStop&) = delete;

                FragmentedChangePitStop& operator=(const FragmentedChangePitStop&) = delete;
            };
        }
    } // namespace fastrtps
} // namespace eprosima

#endif // _RTPS_READER_FRAGMENTEDCHANGEPITSTOP_H_
