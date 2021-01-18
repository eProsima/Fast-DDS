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

#ifndef _FASTDDS_TOPICDATATYPE_HPP_
#define _FASTDDS_TOPICDATATYPE_HPP_

#include <string>
#include <functional>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>

#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/utils/md5.h>

// This version of TypeSupport has `is_bounded()`
#define TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

// This version of TypeSupport has `is_plain()`
#define TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

// This version of TypeSupport has `construct_sample()`
#define TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

namespace eprosima {
namespace fastrtps {

namespace rtps {
struct SerializedPayload_t;
struct InstanceHandle_t;
} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace dds {

class TypeSupport;

/**
 * Class TopicDataType used to provide the DomainRTPSParticipant with the methods to serialize, deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one, where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTRTPS_MODULE, FASTDDS_MODULE
 */
class TopicDataType
{
public:

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TopicDataType()
        : m_typeSize(0)
        , m_isGetKeyDefined(false)
        , auto_fill_type_object_(true)
        , auto_fill_type_information_(true)
    {
    }

    /**
     * @brief Destructor
     */
    RTPS_DllAPI virtual ~TopicDataType()
    {
    }

    /**
     * Serialize method, it should be implemented by the user, since it is abstract.
     * It is VERY IMPORTANT that the user sets the SerializedPayload length correctly.
     * @param[in] data Pointer to the data
     * @param[out] payload Pointer to the payload
     * @return True if correct.
     */
    RTPS_DllAPI virtual bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) = 0;

    /**
     * Deserialize method, it should be implemented by the user, since it is abstract.
     * @param[in] payload Pointer to the payload
     * @param[out] data Pointer to the data
     * @return True if correct.
     */
    RTPS_DllAPI virtual bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data) = 0;

    /**
     * @brief Gets the SerializedSizeProvider function
     * @param data Pointer
     * @return function
     */
    RTPS_DllAPI virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) = 0;

    /**
     * Create a Data Type.
     * @return Void pointer to the created object.
     */
    RTPS_DllAPI virtual void* createData() = 0;
    /**
     * Remove a previously created object.
     * @param data Pointer to the created Data.
     */
    RTPS_DllAPI virtual void deleteData(
            void* data) = 0;

    /**
     * Get the key associated with the data.
     * @param[in] data Pointer to the data.
     * @param[out] ihandle Pointer to the Handle.
     * @param[in] force_md5 Force MD5 checking.
     * @return True if correct.
     */
    RTPS_DllAPI virtual bool getKey(
            void* data,
            fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) = 0;

    /**
     * Set topic data type name
     * @param nam Topic data type name
     */
    RTPS_DllAPI inline void setName(
            const char* nam)
    {
        m_topicDataTypeName = std::string(nam);
    }

    /**
     * Get topic data type name
     * @return Topic data type name
     */
    RTPS_DllAPI inline const char* getName() const
    {
        return m_topicDataTypeName.c_str();
    }

    /**
     * Get the type object auto-fill configuration
     * @return true if the type object should be auto-filled
     */
    RTPS_DllAPI inline bool auto_fill_type_object() const
    {
        return auto_fill_type_object_;
    }

    /**
     * Set the type object auto-fill configuration
     * @param auto_fill_type_object new value to set
     */
    RTPS_DllAPI inline void auto_fill_type_object(
            bool auto_fill_type_object)
    {
        auto_fill_type_object_ = auto_fill_type_object;
    }

    /**
     * Get the type information auto-fill configuration
     * @return true if the type information should be auto-filled
     */
    RTPS_DllAPI inline bool auto_fill_type_information() const
    {
        return auto_fill_type_information_;
    }

    /**
     * Set type information auto-fill configuration
     * @param auto_fill_type_information new value to set
     */
    RTPS_DllAPI inline void auto_fill_type_information(
            bool auto_fill_type_information)
    {
        auto_fill_type_information_ = auto_fill_type_information;
    }

    /**
     * Get the type identifier
     * @return TypeIdV1
     */
    RTPS_DllAPI inline const std::shared_ptr<TypeIdV1> type_identifier() const
    {
        return type_identifier_;
    }

    /**
     * Set type identifier
     * @param id new value for TypeIdV1
     */
    RTPS_DllAPI inline void type_identifier(
            const TypeIdV1& id)
    {
        type_identifier_ = std::make_shared<TypeIdV1>(id);
    }

    /**
     * Set type identifier
     * @param id shared pointer to TypeIdV1
     */
    RTPS_DllAPI inline void type_identifier(
            const std::shared_ptr<TypeIdV1> id)
    {
        type_identifier_ = std::move(id);
    }

    /**
     * Get the type object
     * @return TypeObjectV1
     */
    RTPS_DllAPI inline const std::shared_ptr<TypeObjectV1> type_object() const
    {
        return type_object_;
    }

    /**
     * Set type object
     * @param object new value for TypeObjectV1
     */
    RTPS_DllAPI inline void type_object(
            const TypeObjectV1& object)
    {
        type_object_ = std::make_shared<TypeObjectV1>(object);
    }

    /**
     * Set type object
     * @param object shared pointer to TypeObjectV1
     */
    RTPS_DllAPI inline void type_object(
            std::shared_ptr<TypeObjectV1> object)
    {
        type_object_ = std::move(object);
    }

    /**
     * Get the type information
     * @return TypeInformation
     */
    RTPS_DllAPI inline const std::shared_ptr<xtypes::TypeInformation> type_information() const
    {
        return type_information_;
    }

    /**
     * Set type information
     * @param info new value for TypeInformation
     */
    RTPS_DllAPI inline void type_information(
            const xtypes::TypeInformation& info)
    {
        type_information_ = std::make_shared<xtypes::TypeInformation>(info);
    }

    /**
     * Set type information
     * @param info shared pointer to TypeInformation
     */
    RTPS_DllAPI inline void type_information(
            std::shared_ptr<xtypes::TypeInformation> info)
    {
        type_information_ = std::move(info);
    }

    /**
     * Checks if the type is bounded.
     */
    RTPS_DllAPI virtual inline bool is_bounded() const
    {
        return false;
    }

    /**
     * Checks if the type is plain.
     */
    RTPS_DllAPI virtual inline bool is_plain() const
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
    RTPS_DllAPI virtual inline bool construct_sample(
            void* memory) const
    {
        static_cast<void>(memory);
        return false;
    }

    //! Maximum serialized size of the type in bytes.
    //! If the type has unbounded fields, and therefore cannot have a maximum size, use 0.
    uint32_t m_typeSize;

    //! Indicates whether the method to obtain the key has been implemented.
    bool m_isGetKeyDefined;

protected:

    //!Type Identifier XTYPES 1.1
    std::shared_ptr<TypeIdV1> type_identifier_;
    //!Type Object XTYPES 1.1
    std::shared_ptr<TypeObjectV1> type_object_;
    //!XTYPES 1.2
    std::shared_ptr<xtypes::TypeInformation> type_information_;

private:

    //! Data Type Name.
    std::string m_topicDataTypeName;

    bool auto_fill_type_object_;
    bool auto_fill_type_information_;

    friend class fastdds::dds::TypeSupport;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_TOPICDATATYPE_HPP_ */
