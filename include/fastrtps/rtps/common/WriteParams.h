/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriteParams.h
 */
#ifndef _FASTRTPS_RTPS_COMMON_WRITEPARAMS_H_
#define _FASTRTPS_RTPS_COMMON_WRITEPARAMS_H_

#include "SampleIdentity.h"

namespace eprosima
{
    namespace fastrtps
    {
        namespace rtps
        {
            /*!
             * @brief This class contains additional information of a CacheChange.
             * @ingroup COMMON_MODULE
             */
            class WriteParams
            {
                public:

                    /*!
                     * @brief Default constructor.
                     */
                    WriteParams()
                    {
                    }

                    /*!
                     * @brief Copy constructor.
                     */
                    WriteParams(const WriteParams &wparam) : sample_identity_(wparam.sample_identity_),
                    related_sample_identity_(wparam.related_sample_identity_)
                    {
                    }

                    /*!
                     * @brief Move constructor.
                     */
                    WriteParams(WriteParams &&wparam) : sample_identity_(std::move(wparam.sample_identity_)),
                    related_sample_identity_(std::move(wparam.related_sample_identity_))
                    {
                    }

                    /*!
                     * @brief Assignment operator
                     */
                    WriteParams& operator=(const WriteParams &wparam)
                    {
                        sample_identity_ = wparam.sample_identity_;
                        related_sample_identity_ = wparam.related_sample_identity_;
                        return *this;
                    }

                    /*!
                     * @brief Assignment operator
                     */
                    WriteParams& operator=(WriteParams &&wparam)
                    {
                        sample_identity_ = std::move(wparam.sample_identity_);
                        related_sample_identity_ = std::move(wparam.related_sample_identity_);
                        return *this;
                    }

                    WriteParams& sample_identity(const SampleIdentity &sample_id)
                    {
                        sample_identity_ = sample_id;
                        return *this;
                    }

                    WriteParams& sample_identity(SampleIdentity &&sample_id)
                    {
                        sample_identity_ = std::move(sample_id);
                        return *this;
                    }

                    const SampleIdentity& sample_identity() const
                    {
                        return sample_identity_;
                    }

                    SampleIdentity& sample_identity()
                    {
                        return sample_identity_;
                    }

                    WriteParams& related_sample_identity(const SampleIdentity &sample_id)
                    {
                        related_sample_identity_ = sample_id;
                        return *this;
                    }

                    WriteParams& related_sample_identity(SampleIdentity &&sample_id)
                    {
                        related_sample_identity_ = std::move(sample_id);
                        return *this;
                    }

                    const SampleIdentity& related_sample_identity() const
                    {
                        return related_sample_identity_;
                    }

                    SampleIdentity& related_sample_identity()
                    {
                        return related_sample_identity_;
                    }

                private:

                    SampleIdentity sample_identity_;

                    SampleIdentity related_sample_identity_;
            };

        } //namespace rtps
    } //namespace fastrtps
} //namespace eprosima
#endif //_FASTRTPS_RTPS_COMMON_WRITEPARAMS_H_
