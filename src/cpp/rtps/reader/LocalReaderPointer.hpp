// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LocalReaderPointer.hpp
 */
#ifndef FASTDDS_RTPS_READER__LOCALREADERPOINTER_HPP
#define FASTDDS_RTPS_READER__LOCALREADERPOINTER_HPP

#include <memory>

#include <fastdds/rtps/Endpoint.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader;
class LocalReaderView;
class WeakLocalReaderPointer;

/**
 * @brief Class representing a pointer to a local reader.
 *
 * This class simply encapsulates a pointer to a BaseReader and a
 * shared pointer to its associated LocalReaderView.
 *
 * @ingroup READER_MODULE
 */
class WeakLocalReaderPointer
{
    friend class LocalReaderPointer;

public:

    /**
     * @brief Default constructor for WeakLocalReaderPointer.
     *
     * Initializes an empty WeakLocalReaderPointer.
     */
    WeakLocalReaderPointer();

    /**
     * @brief Constructs a WeakLocalReaderPointer with a specified reader and view.
     *
     * @param reader Pointer to a BaseReader instance.
     * @param view Shared pointer to a LocalReaderView.
     */
    WeakLocalReaderPointer(
            BaseReader* reader,
            std::shared_ptr<LocalReaderView> view);

    /**
     * @brief WeakLocalpointers cannot be copied.
     */
    WeakLocalReaderPointer(
            const WeakLocalReaderPointer&) = default;

    /**
     * @brief Destructor for WeakLocalReaderPointer.
     *
     * Cleans up the resources held by the WeakLocalReaderPointer.
     */
    ~WeakLocalReaderPointer() = default;

    /**
     * @brief Overloaded operator-> to access members of BaseReader.
     *
     * @return Pointer to the associated BaseReader, enabling member access.
     */
    BaseReader* operator ->();

    /**
     * @brief Overloaded assignment operator.
     *
     * @param other WeakLocalReaderPointer to assign.
     * @return Reference to the assigned WeakLocalReaderPointer.
     */
    WeakLocalReaderPointer& operator =(
            const WeakLocalReaderPointer& other) = default;

    /**
     * @brief Overloaded operator bool to check the status of the reader.
     *
     * @return True if the reader is active.
     */
    explicit operator bool() const;

    /**
     * @brief Resets the WeakLocalReaderPointer.
     */
    void reset();

protected:

    BaseReader* local_reader_{nullptr};
    std::shared_ptr<LocalReaderView> view_;
};


/**
 * @brief Class representing a pointer to a local reader.
 *
 * This class encapsulates a pointer to a BaseReader and manages
 * the associated LocalReaderView of it by increasing the reference counter
 * on construction and copy, and dereferencing upon destruction.
 *
 * The difference with the WeakLocalReaderPointer is that this
 * class manages the reference counter of the reader's view.
 *
 * This class is meant to be used before as a temporal stack variable
 * before accessing any of the methods of the contained reader i.e
 * holding an instance of this class as a member is discouraged, instead
 * use a WeakLocalReaderPointer in that case.
 *
 * @ingroup READER_MODULE
 */
class LocalReaderPointer : public WeakLocalReaderPointer
{
public:

    /**
     * @brief Default constructor for LocalReaderPointer.
     *
     * Initializes an empty LocalReaderPointer.
     */
    LocalReaderPointer();

    /**
     * @brief Constructs a LocalReaderPointer with a specified reader and view.
     *
     * @param reader Pointer to a BaseReader instance.
     * @param view Shared pointer to a LocalReaderView.
     */
    LocalReaderPointer(
            BaseReader* reader,
            std::shared_ptr<LocalReaderView> view);

    /**
     * @brief copy constructor
     */
    LocalReaderPointer(
            const LocalReaderPointer&);

    /**
     * @brief Construction from a WeakLocalReaderPointer.
     */
    LocalReaderPointer(
            const WeakLocalReaderPointer&);

    /**
     * @brief Destructor for LocalReaderPointer.
     *
     * Cleans up the resources held by the LocalReaderPointer.
     */
    ~LocalReaderPointer();

    /**
     * @brief Overloaded operator-> to access members of BaseReader.
     *
     * @return Pointer to the associated BaseReader, enabling member access.
     */
    BaseReader* operator ->();

    /**
     * @brief Overloaded assignment operator.
     *
     * @param other LocalReaderPointer to assign.
     * @return Reference to the assigned LocalReaderPointer.
     */
    LocalReaderPointer& operator =(
            const LocalReaderPointer& other);

    /**
     * @brief Overloaded assignment operator.
     *
     * @param other WeakLocalReaderPointer to assign.
     * @return Reference to the assigned LocalReaderPointer.
     */
    LocalReaderPointer& operator =(
            const WeakLocalReaderPointer& other);

    /**
     * @brief Overloaded operator bool to check the status of the reader.
     *
     * @return True if the reader is active.
     */
    explicit operator bool() const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__LOCALREADERPOINTER_HPP
