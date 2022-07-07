// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeIntrospectionPublisher.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONPUBLISHER_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONPUBLISHER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "types/types.hpp"

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class TypeIntrospectionPublisher : public eprosima::fastdds::dds::DomainParticipantListener
{
public:

    ///////////////////////////////////////////
    // Constructors and destructors
    ///////////////////////////////////////////

    TypeIntrospectionPublisher(
            const std::string& topic_name,
            const uint32_t domain,
            DataTypeKind data_type_kind,
            GeneratorKind generator_kind,
            bool use_type_object,
            bool use_type_information);

    virtual ~TypeIntrospectionPublisher();


    ///////////////////////////////////////////
    // Interaction methods
    ///////////////////////////////////////////

    //! Run for number samples, publish every sleep seconds
    void run(
            uint32_t number,
            uint32_t sleep);


    ///////////////////////////////////////////
    // Listener callback methods
    ///////////////////////////////////////////

    void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;


    ///////////////////////////////////////////
    // Static methods
    ///////////////////////////////////////////

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

protected:

    ///////////////////////////////////////////
    // Auxiliar internal methods
    ///////////////////////////////////////////

    /**
     * @brief Publish a sample
     *
     * This method creates each time a variable of type \c data_type_ , initializes it
     * depending on the \c msg_index and publishes it.
     */
    void publish(unsigned int msg_index);

    //! Run thread for number samples, publish every sleep seconds
    void runThread(
            uint32_t number,
            uint32_t sleep);


    ///////////////////////////////////////////
    // Data Type
    ///////////////////////////////////////////

    // Data Type manager object
    std::unique_ptr<IDataType> data_type_;


    ///////////////////////////////////////////
    // Fast DDS entities
    ///////////////////////////////////////////

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;


    ///////////////////////////////////////////
    // Static variables
    ///////////////////////////////////////////

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;
};

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_TYPEINTROSPECTIONEXAMPLE_TYPEINTROSPECTIONPUBLISHER_H_ */
