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

#ifndef __TRANSPORT_EPROSIMASHAREDMEM_HPP__
#define __TRANSPORT_EPROSIMASHAREDMEM_HPP__

namespace eprosima {
namespace fastdds {
namespace rtps {

class PortObserver
{
public:
	virtual void on_write(const octet* buffer, uint32_t size) = 0;
};


class Port
{
public:

	Port(const Locator_t& locator)
		: locator_(locator)
	{
printf("eProsimaSharedMem New Port - %d.%d.%d.%d:%u\n", locator.address[12], locator.address[13], locator.address[14], locator.address[15], locator.port);
	}

	const Locator_t& locator() const
	{
		return locator_;
	}

	void add_observer(std::shared_ptr<PortObserver> observer)
	{
printf("eProsimaSharedMem Port - %d.%d.%d.%d:%u Observers %d\n", locator_.address[12], locator_.address[13], locator_.address[14], locator_.address[15], locator_.port, observers_.size()+1);
		observers_.push_back(observer);
	}

	void write(const octet* buffer, uint32_t size, const std::chrono::microseconds& timeout)
	{
		// TODO: Handle timeout
		for (auto& observer : observers_)
		{
			observer->on_write(buffer, size);
		}
	}

private:

	std::vector<std::shared_ptr<PortObserver>> observers_;
	Locator_t locator_;
};

class eProsimaSharedMem
{
public:

	enum class ChannelType {READER, WRITER};

	eProsimaSharedMem(ChannelType channel_type, const Locator_t& locator, uint32_t max_msg_size)
	{
		channel_type_ = channel_type;
		locator_ = locator;
		max_msg_size_ = max_msg_size;

		if (channel_type == ChannelType::WRITER)
		{
			// Nothing to do
			//writer_port_ = create_port(locator);
		}
		else // Is ChannelType::READER
		{
			observe_port(locator);
		}
	}

	Locator_t reading_locator() const
	{
		assert(channel_type_ == ChannelType::READER);
		return locator_;
	}

	int read(octet* buffer, uint32_t capacity)
	{
		return port_reader_->read(buffer, capacity);
	}
	
	void write(const octet* buffer, uint32_t size, const Locator_t& locator, const std::chrono::microseconds& timeout)
	{
		std::lock_guard<std::mutex> lock(global_ports_mutex_);

		for (auto port : global_ports_)
		{
			if (port->locator() == locator)
			{
				port->write(buffer, size, timeout);
				break;
			}
		}
	}

	void cancel()
	{

	}
	void close()
	{
		// must Cancel all asynchronous operations.
	}

private:

	ChannelType channel_type_;
	Locator_t locator_;
	uint32_t max_msg_size_;

	// Reader Exclusive ******************************
	class PortReader : public PortObserver
	{
	public:
		PortReader(uint32_t max_msg_size)
			:	message_size_(0),
				max_message_size_(max_msg_size)
		{
			message_buffer_ = std::unique_ptr<uint8_t[]>(new uint8_t[max_msg_size]);
		}

		int read(octet* buffer, uint32_t capacity)
		{
			std::unique_lock<std::mutex> lock(write_cv_mutex_);

			write_cv_.wait(lock, [&] {return message_size_ != 0; });

			if (message_size_ > capacity)
				return 0; // TODO: Check this

			int bytes_written = message_size_;
			memcpy(buffer, message_buffer_.get(), message_size_);
			message_size_ = 0; // signal buffer empty

			return bytes_written;
		}

	private:

		void on_write(const octet* buffer, uint32_t size) override
		{
			std::unique_lock<std::mutex> lock(write_cv_mutex_);

			if (size > max_message_size_)
				throw - 1; // TODO: Implement exceptions

			memcpy(message_buffer_.get(), buffer, size);
			message_size_ = size;

			lock.unlock();
			write_cv_.notify_one();
		}

		std::mutex write_cv_mutex_;
		std::condition_variable write_cv_;
		uint32_t message_size_;
		std::unique_ptr<uint8_t[]> message_buffer_;

		uint32_t max_message_size_;
	};

	std::shared_ptr<PortReader> port_reader_;

	void observe_port(const Locator_t& locator)
	{
		std::lock_guard<std::mutex> lock(global_ports_mutex_);

		std::shared_ptr<Port> port;

		// Find the port
		auto port_it = std::find_if(global_ports_.begin(), global_ports_.end(), [&](std::shared_ptr<Port> port) { return locator == port->locator(); });
		if (port_it == global_ports_.end())
		{
			// doesn't exist => create the port
			port = std::make_shared<Port>(locator);
			global_ports_.push_back(port);
		}
		else
			port = (*port_it);

		port_reader_ = std::make_shared<PortReader>(max_msg_size_);
		port->add_observer(port_reader_);
	}

	//**************************************

	static std::vector<std::shared_ptr<Port>> global_ports_;
	static std::mutex global_ports_mutex_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // __TRANSPORT_EPROSIMASHAREDMEM_HPP__
