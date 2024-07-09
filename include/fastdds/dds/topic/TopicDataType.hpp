// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TopicDataType.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP
#define FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP

#include <functional>
#include <memory>
#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/utils/md5.hpp>

// This version of TypeSupport has `is_bounded()`
#define TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

// This version of TypeSupport has `is_plain()`
#define TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

// This version of TypeSupport has `construct_sample()`
#define TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

namespace eprosima {
namespace fastdds {

namespace rtps {
struct SerializedPayload_t;
struct InstanceHandle_t;
} // namespace rtps

namespace dds {

class TypeSupport;

/**
 * Class TopicDataType used to provide the DomainRTPSParticipant with the methods to serialize, deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one, where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTDDS_MODULE
 */
class TopicDataType
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API TopicDataType();

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~TopicDataType();

    /**
     * Serialize method, it should be implemented by the user, since it is abstract.
     * It is VERY IMPORTANT that the user sets the SerializedPayload length correctly.
     *
     * @param [in] data Pointer to the data
     * @param [out] payload Pointer to the payload
     * @return True if correct.
     */
    // TODO(jlbueno) Remove when Fast DDS-Gen is updated
    // FASTDDS_TODO_BEFORE(3, 0, "Remove this overload")
    FASTDDS_EXPORTED_API virtual bool serialize(
            const void* const data,
            fastdds::rtps::SerializedPayload_t* payload) = 0;

    /**
     * Serialize method, it should be implemented by the user, since it is abstract. If not implemented, this method
     * will call the serialize method in which the topic data representation is not considered.
     * It is VERY IMPORTANT that the user sets the SerializedPayload length correctly.
     *
     * @param [in] data Pointer to the data
     * @param [out] payload Pointer to the payload
     * @param [in] data_representation Representation that should be used to encode the data into the payload.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool serialize(
            const void* const data,
            fastdds::rtps::SerializedPayload_t* payload,
            DataRepresentationId_t data_representation);

    /**
     * Deserialize method, it should be implemented by the user, since it is abstract.
     *
     * @param [in] payload Pointer to the payload
     * @param [out] data Pointer to the data
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool deserialize(
            fastdds::rtps::SerializedPayload_t* payload,
            void* data) = 0;

    /*!
     * @brief Returns a function which can be used to calculate the serialized size of the provided data.
     *
     * @param [in] data Pointer to data.
     * @return Functor which calculates the serialized size of the data.
     */
    // FASTDDS_TODO_BEFORE(3, 0, "Remove this overload")
    FASTDDS_EXPORTED_API virtual std::function<uint32_t()> getSerializedSizeProvider(
            const void* const data) = 0;

    /*!
     * @brief Returns a function which can be used to calculate the serialized size of the provided data.
     *
     * @param [in] data Pointer to data.
     * @param [in] data_representation Representation that should be used for calculating the serialized size.
     * @return Functor which calculates the serialized size of the data.
     */
    FASTDDS_EXPORTED_API virtual std::function<uint32_t()> getSerializedSizeProvider(
            const void* const data,
            DataRepresentationId_t data_representation);

    /**
     * Create a Data Type.
     *
     * @return Void pointer to the created object.
     */
    FASTDDS_EXPORTED_API virtual void* createData() = 0;
    /**
     * Remove a previously created object.
     *
     * @param data Pointer to the created Data.
     */
    FASTDDS_EXPORTED_API virtual void deleteData(
            void* data) = 0;

    /**
     * Get the key associated with the data.
     *
     * @param [in] data Pointer to the data.
     * @param [out] ihandle Pointer to the Handle.
     * @param [in] force_md5 Force MD5 checking.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool getKey(
            const void* const data,
            fastdds::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) = 0;

    /**
     * Set topic data type name
     *
     * @param nam Topic data type name
     */
    FASTDDS_EXPORTED_API inline void setName(
            const char* nam)
    {
        m_topicDataTypeName = std::string(nam);
    }

    /**
     * Get topic data type name
     *
     * @return Topic data type name
     */
    FASTDDS_EXPORTED_API inline const char* getName() const
    {
        return m_topicDataTypeName.c_str();
    }

    /**
     * Get the type information auto-fill configuration
     *
     * @return true if the type information should be auto-filled
     */
    FASTDDS_EXPORTED_API inline bool auto_fill_type_information() const
    {
        return auto_fill_type_information_;
    }

    /**
     * Set type information auto-fill configuration
     *
     * @param auto_fill_type_information new value to set
     */
    FASTDDS_EXPORTED_API inline void auto_fill_type_information(
            bool auto_fill_type_information)
    {
        auto_fill_type_information_ = auto_fill_type_information;
    }

    /**
     * Get the type identifiers
     *
     * @return @ref xtypes::TypeIdentifierPair
     */
    FASTDDS_EXPORTED_API inline const xtypes::TypeIdentifierPair& type_identifiers() const
    {
        return type_identifiers_;
    }

    /**
     * Checks if the type is bounded.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_bounded() const
    {
        return false;
    }

    /**
     * Checks if the type is plain when using default encoding.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_plain() const
    {
        return false;
    }

    /**
     * Checks if the type is plain when using a specific encoding.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_plain(
            DataRepresentationId_t) const
    {
        return false;
    }

    /**
     * Construct a sample on a memory location.
     *
     * @param memory Pointer to the memory location where the sample should be constructed.
     *
     * @return whether this type supports in-place construction or not.
     */
    FASTDDS_EXPORTED_API virtual inline bool construct_sample(
            void* memory) const
    {
        static_cast<void>(memory);
        return false;
    }

    /**
     * @brief Register TypeObject type representation
     */
    FASTDDS_EXPORTED_API virtual inline void register_type_object_representation()
    {
    }

    //! Maximum serialized size of the type in bytes.
    //! If the type has unbounded fields, and therefore cannot have a maximum size, use 0.
    uint32_t m_typeSize;

    //! Indicates whether the method to obtain the key has been implemented.
    bool m_isGetKeyDefined;

protected:

    xtypes::TypeIdentifierPair type_identifiers_;

private:

    //! Data Type Name.
    std::string m_topicDataTypeName;
    //TODO(XTypes)
    bool auto_fill_type_information_;

    friend class fastdds::dds::TypeSupport;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP
