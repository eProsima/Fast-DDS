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

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader;
class LocalReaderView;

/**
 * @brief Class representing a pointer to a local reader.
 *
 * This class encapsulates a pointer to a BaseReader and manages
 * the associated LocalReaderView of it.
 * This permits to query the entity status before accessing the reader.
 * @ingroup READER_MODULE
 */
class LocalReaderPointer
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
     * @brief Destructor for LocalReaderPointer.
     *
     * Cleans up the resources held by the LocalReaderPointer.
     */
    ~LocalReaderPointer();

    /**
     * @brief Retrieves the associated BaseReader pointer.
     *
     * @return Pointer to the associated BaseReader.
     */
    BaseReader* reader();

    /**
     * @brief Overloaded operator-> to access members of BaseReader.
     *
     * @return Pointer to the associated BaseReader, enabling member access.
     */
    BaseReader* operator->();

    /**
     * @brief Sets the local reader pointer.
     *
     * @param reader Pointer to a BaseReader instance.
     */
    inline void local_reader(
            BaseReader* reader)
    {
        local_reader_ = reader;
    }

    /**
     * @brief Sets the local reader view.
     *
     * @param view Shared pointer to a LocalReaderView.
     */
    inline void local_reader_view(
            std::shared_ptr<LocalReaderView> view)
    {
        view_ = view;
    }

    /**
     * @brief Checks if the local reader pointer is valid.
     *
     * @return True if the different from INACTIVE.
     */
    bool is_valid();

protected:

    BaseReader* local_reader_{nullptr};
    std::weak_ptr<LocalReaderView> view_;
    bool already_referenced_{false};
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_WRITER__LOCALREADERPOINTER_HPP