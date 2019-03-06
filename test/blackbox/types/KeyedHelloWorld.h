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
 * @file KeyedHelloWorld.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 */

#ifndef _KeyedHelloWorld_H_
#define _KeyedHelloWorld_H_

#include <stdint.h>
#include <array>
#include <string>
#include <vector>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif
#else
#define eProsima_user_DllExport
#endif

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(deadlinemessage_SOURCE)
#define deadlinemessage_DllAPI __declspec( dllexport )
#else
#define deadlinemessage_DllAPI __declspec( dllimport )
#endif // deadlinemessage_SOURCE
#else
#define deadlinemessage_DllAPI
#endif
#else
#define deadlinemessage_DllAPI
#endif // _WIN32

namespace eprosima
{
    namespace fastcdr
    {
        class Cdr;
    }
}

/*!
 * @brief This class represents the structure KeyedHelloWorld defined by the user in the IDL file.
 * @ingroup DEADLINEmessage
 */
class KeyedHelloWorld
{
public:

    /*!
     * @brief Default constructor.
     */
    eProsima_user_DllExport KeyedHelloWorld();
    
    /*!
     * @brief Default destructor.
     */
    eProsima_user_DllExport ~KeyedHelloWorld();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object KeyedHelloWorld that will be copied.
     */
    eProsima_user_DllExport KeyedHelloWorld(const KeyedHelloWorld &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object KeyedHelloWorld that will be copied.
     */
    eProsima_user_DllExport KeyedHelloWorld(KeyedHelloWorld &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object KeyedHelloWorld that will be copied.
     */
    eProsima_user_DllExport KeyedHelloWorld& operator=(const KeyedHelloWorld &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object KeyedHelloWorld that will be copied.
     */
    eProsima_user_DllExport KeyedHelloWorld& operator=(KeyedHelloWorld &&x);
    
    eProsima_user_DllExport bool operator==(const KeyedHelloWorld &x) const;

    /*!
     * @brief This function sets a value in member deadlinekey
     * @param _deadlinekey New value for member deadlinekey
     */
    inline eProsima_user_DllExport void key(uint16_t _deadlinekey)
    {
        m_key = _deadlinekey;
    }

    /*!
     * @brief This function returns the value of member deadlinekey
     * @return Value of member deadlinekey
     */
    inline eProsima_user_DllExport uint16_t key() const
    {
        return m_key;
    }

    /*!
     * @brief This function returns a reference to member deadlinekey
     * @return Reference to member deadlinekey
     */
    inline eProsima_user_DllExport uint16_t& key()
    {
        return m_key;
    }
    /*!
     * @brief This function copies the value in member message
     * @param _message New value to be copied in member message
     */
    inline eProsima_user_DllExport void message(const std::string &_message)
    {
        m_message = _message;
    }

    /*!
     * @brief This function moves the value in member message
     * @param _message New value to be moved in member message
     */
    inline eProsima_user_DllExport void message(std::string &&_message)
    {
        m_message = std::move(_message);
    }

    /*!
     * @brief This function returns a constant reference to member message
     * @return Constant reference to member message
     */
    inline eProsima_user_DllExport const std::string& message() const
    {
        return m_message;
    }

    /*!
     * @brief This function returns a reference to member message
     * @return Reference to member message
     */
    inline eProsima_user_DllExport std::string& message()
    {
        return m_message;
    }
    
    /*!
     * @brief This function returns the maximum serialized size of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);

    /*!
     * @brief This function returns the serialized size of a data depending on the buffer alignment.
     * @param data Data which is calculated its serialized size.
     * @param current_alignment Buffer alignment.
     * @return Serialized size.
     */
    eProsima_user_DllExport static size_t getCdrSerializedSize(const KeyedHelloWorld& data, size_t current_alignment = 0);


    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serialize(eprosima::fastcdr::Cdr &cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void deserialize(eprosima::fastcdr::Cdr &cdr);



    /*!
     * @brief This function returns the maximum serialized size of the Key of an object
     * depending on the buffer alignment.
     * @param current_alignment Buffer alignment.
     * @return Maximum serialized size.
     */
    eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(size_t current_alignment = 0);

    /*!
     * @brief This function tells you if the Key has been defined for this type
     */
    eProsima_user_DllExport static bool isKeyDefined();

    /*!
     * @brief This function serializes the key members of an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    eProsima_user_DllExport void serializeKey(eprosima::fastcdr::Cdr &cdr) const;
    
private:
    uint16_t m_key;
    std::string m_message;
};

#endif // _KeyedHelloWorld_H_
