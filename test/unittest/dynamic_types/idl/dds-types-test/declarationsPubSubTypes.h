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

/*!
 * @file declarationsPubSubTypes.h
 * This header file contains the declaration of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#ifndef _FAST_DDS_GENERATED_DECLARATIONS_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_DECLARATIONS_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/utils/md5.h>

#include "declarations.h"


#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    Generated declarations is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

class ForwardStruct;

class ForwardUnion;

typedef std::vector<ForwardStruct> RecursiveUnboundedSeqForwardStruct;
typedef std::vector<ForwardStruct> RecursiveBoundedSeqForwardStruct;
typedef std::vector<ForwardUnion> RecursiveUnboundedSeqForwardUnion;
typedef std::vector<ForwardUnion> RecursiveBoundedSeqForwardUnion;

/*!
 * @brief This class represents the TopicDataType of the type ForwardDeclarationsStruct defined by the user in the IDL file.
 * @ingroup declarations
 */
class ForwardDeclarationsStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef ForwardDeclarationsStruct type;

    eProsima_user_DllExport ForwardDeclarationsStructPubSubType();

    eProsima_user_DllExport virtual ~ForwardDeclarationsStructPubSubType() override;

    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport virtual void* createData() override;

    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        (void)memory;
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};

/*!
 * @brief This class represents the TopicDataType of the type ForwardDeclarationsRecursiveStruct defined by the user in the IDL file.
 * @ingroup declarations
 */
class ForwardDeclarationsRecursiveStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef ForwardDeclarationsRecursiveStruct type;

    eProsima_user_DllExport ForwardDeclarationsRecursiveStructPubSubType();

    eProsima_user_DllExport virtual ~ForwardDeclarationsRecursiveStructPubSubType() override;

    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport virtual void* createData() override;

    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        (void)memory;
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};


#ifndef SWIG
namespace detail {

    template<typename Tag, typename Tag::type M>
    struct ForwardStruct_rob
    {
        friend constexpr typename Tag::type get(
                Tag)
        {
            return M;
        }
    };

    struct ForwardStruct_f
    {
        typedef int32_t ForwardStruct::* type;
        friend constexpr type get(
                ForwardStruct_f);
    };

    template struct ForwardStruct_rob<ForwardStruct_f, &ForwardStruct::m_var_long>;

    template <typename T, typename Tag>
    inline size_t constexpr ForwardStruct_offset_of() {
        return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
    }
}
#endif

/*!
 * @brief This class represents the TopicDataType of the type ForwardStruct defined by the user in the IDL file.
 * @ingroup declarations
 */
class ForwardStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef ForwardStruct type;

    eProsima_user_DllExport ForwardStructPubSubType();

    eProsima_user_DllExport virtual ~ForwardStructPubSubType() override;

    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport virtual void* createData() override;

    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return is_plain_impl();
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        new (memory) ForwardStruct();
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

private:

    static constexpr bool is_plain_impl()
    {
        return 8ULL == (detail::ForwardStruct_offset_of<ForwardStruct, detail::ForwardStruct_f>() + sizeof(int32_t));

    }};
namespace declarations_module
{
    class ModuledForwardStruct;

    class ModuledForwardUnion;

    typedef std::vector<declarations_module::ModuledForwardStruct> ModuledRecursiveUnboundedSeqForwardStruct;
    typedef std::vector<declarations_module::ModuledForwardStruct> ModuledRecursiveBoundedSeqForwardStruct;
    typedef std::vector<declarations_module::ModuledForwardUnion> ModuledRecursiveUnboundedSeqForwardUnion;
    typedef std::vector<declarations_module::ModuledForwardUnion> ModuledRecursiveBoundedSeqForwardUnion;
}

/*!
 * @brief This class represents the TopicDataType of the type ModuledForwardDeclarationsStruct defined by the user in the IDL file.
 * @ingroup declarations
 */
class ModuledForwardDeclarationsStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef ModuledForwardDeclarationsStruct type;

    eProsima_user_DllExport ModuledForwardDeclarationsStructPubSubType();

    eProsima_user_DllExport virtual ~ModuledForwardDeclarationsStructPubSubType() override;

    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport virtual void* createData() override;

    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        (void)memory;
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};

/*!
 * @brief This class represents the TopicDataType of the type ModuledForwardDeclarationsRecursiveStruct defined by the user in the IDL file.
 * @ingroup declarations
 */
class ModuledForwardDeclarationsRecursiveStructPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef ModuledForwardDeclarationsRecursiveStruct type;

    eProsima_user_DllExport ModuledForwardDeclarationsRecursiveStructPubSubType();

    eProsima_user_DllExport virtual ~ModuledForwardDeclarationsRecursiveStructPubSubType() override;

    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport virtual void* createData() override;

    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        (void)memory;
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};
namespace declarations_module
{

    #ifndef SWIG
    namespace detail {

        template<typename Tag, typename Tag::type M>
        struct ModuledForwardStruct_rob
        {
            friend constexpr typename Tag::type get(
                    Tag)
            {
                return M;
            }
        };

        struct ModuledForwardStruct_f
        {
            typedef int32_t ModuledForwardStruct::* type;
            friend constexpr type get(
                    ModuledForwardStruct_f);
        };

        template struct ModuledForwardStruct_rob<ModuledForwardStruct_f, &ModuledForwardStruct::m_var_long>;

        template <typename T, typename Tag>
        inline size_t constexpr ModuledForwardStruct_offset_of() {
            return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
        }
    }
    #endif

    /*!
     * @brief This class represents the TopicDataType of the type ModuledForwardStruct defined by the user in the IDL file.
     * @ingroup declarations
     */
    class ModuledForwardStructPubSubType : public eprosima::fastdds::dds::TopicDataType
    {
    public:

        typedef ModuledForwardStruct type;

        eProsima_user_DllExport ModuledForwardStructPubSubType();

        eProsima_user_DllExport virtual ~ModuledForwardStructPubSubType() override;

        eProsima_user_DllExport virtual bool serialize(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

        eProsima_user_DllExport virtual bool deserialize(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void* data) override;

        eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                void* data) override;

        eProsima_user_DllExport virtual bool getKey(
                void* data,
                eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                bool force_md5 = false) override;

        eProsima_user_DllExport virtual void* createData() override;

        eProsima_user_DllExport virtual void deleteData(
                void* data) override;

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
        eProsima_user_DllExport inline bool is_bounded() const override
        {
            return true;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
        eProsima_user_DllExport inline bool is_plain() const override
        {
            return is_plain_impl();
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
        eProsima_user_DllExport inline bool construct_sample(
                void* memory) const override
        {
            new (memory) ModuledForwardStruct();
            return true;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

        MD5 m_md5;
        unsigned char* m_keyBuffer;

    private:

        static constexpr bool is_plain_impl()
        {
            return 8ULL == (detail::ModuledForwardStruct_offset_of<ModuledForwardStruct, detail::ModuledForwardStruct_f>() + sizeof(int32_t));

        }};

}

#endif // _FAST_DDS_GENERATED_DECLARATIONS_PUBSUBTYPES_H_
