// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This program is commercial software licensed under the terms of the
// eProsima Software License Agreement Rev 03 (the "License")
//
// You may obtain a copy of the License at
// https://www.eprosima.com/licenses/LICENSE-REV03

#ifndef FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_
#define FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <fastcdr/Cdr.h>
#include <fastcdr/CdrSizeCalculator.hpp>

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/utils/buffer/buffer.hpp>
#include <fastdds/utils/buffer/BufferBackend.hpp>

namespace eprosima {
namespace fastdds {

/**
 * @brief A CdrContext that is aware of eprosima::fastdds::Buffer and can serialize/deserialize it.
 *
 * It is also a registry for BufferBackend instances, allowing different backends to be registered
 * and used for serialization/deserialization.
 */
struct BufferAwareContext : public dds::TopicDataType::Context
{
    FASTDDS_EXPORTED_API ~BufferAwareContext() override = default;

    /**
     * @brief Registers a BufferBackend for a specific backend type.
     *
     * @param backend_type  The type of the backend (e.g., "cpu", "cuda", "demo").
     * @param backend       A shared pointer to the BufferBackend instance to register.
     */
    FASTDDS_EXPORTED_API void register_backend(
            const std::string& backend_type,
            const std::shared_ptr<BufferBackend>& backend);

    /**
     * @brief Retrieves a registered BufferBackend for a specific backend type.
     *
     * @param backend_type  The type of the backend to retrieve.
     *
     * @return A shared pointer to the registered BufferBackend instance, or nullptr if not found.
     */
    FASTDDS_EXPORTED_API std::shared_ptr<BufferBackend> get_backend(
            const std::string& backend_type) const;

    /**
     * @brief Iterates over all registered backends and applies a provided function to each.
     *
     * @param func  A callable that takes two parameters:
     *              - the backend type (std::string).
     *              - a shared pointer to the BufferBackend.
     */
    template<typename _Func>
    inline void for_each_backend(
            _Func&& func) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : backends_)
        {
            func(pair.first, pair.second);
        }
    }

    /**
     * @brief Calculates the serialized size of a Buffer<uint8_t> instance.
     *
     * @param calculator The CdrSizeCalculator to use for size calculation.
     * @param data The Buffer<uint8_t> instance whose serialized size is to be calculated.
     * @param current_alignment The current alignment for serialization.
     *
     * @return The serialized size of the Buffer<uint8_t> instance.
     */
    FASTDDS_EXPORTED_API size_t calculate_serialized_size(
            eprosima::fastcdr::CdrSizeCalculator& calculator,
            const eprosima::fastdds::Buffer<uint8_t>& data,
            size_t& current_alignment) const;

    /**
     * @brief Serializes a Buffer<uint8_t> instance into a Cdr stream.
     *
     * @param cdr The Cdr stream to serialize into.
     * @param data The Buffer<uint8_t> instance to serialize.
     */
    FASTDDS_EXPORTED_API void serialize(
            eprosima::fastcdr::Cdr& cdr,
            const eprosima::fastdds::Buffer<uint8_t>& data) const;

    /**
     * @brief Deserializes a Buffer<uint8_t> instance from a Cdr stream.
     *
     * @param cdr The Cdr stream to deserialize from.
     * @param data The Buffer<uint8_t> instance to deserialize into.
     */
    FASTDDS_EXPORTED_API void deserialize(
            eprosima::fastcdr::Cdr& cdr,
            eprosima::fastdds::Buffer<uint8_t>& data) const;

private:

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<BufferBackend>> backends_;
};

}  // namespace fastdds
}  // namespace eprosima

#endif  // FASTDDS_UTILS_BUFFER__BUFFERAWARECONTEXT_HPP_
