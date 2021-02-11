#pragma once

#include <stdexcept>

namespace glacier
{
	class Buffer
	{
	public:
		/**
		 * @brief Constructor
		 * @param size Size in bytes of the buffer
		*/
		Buffer(size_t size)
			: m_Size(size)
		{
			m_Data = new char[size];
			clear();
		}

		/**
		 * @brief Copy constructor
		 * @param buffer The buffer to copy
		*/
		Buffer(const Buffer& buffer)
		{
			m_Size = buffer.m_Size;
			m_Data = new char[buffer.m_Size];
			memcpy(m_Data, buffer.m_Data, buffer.m_Size);
		}

		/**
		 * @brief Move constructor
		 * @param buffer The buffer to move
		*/
		Buffer(Buffer&& buffer) noexcept
		{
			m_Data = buffer.m_Data;
			buffer.m_Data = nullptr;

			m_Size = buffer.m_Size;
			buffer.m_Size = 0;
		}

		~Buffer()
		{
			if (m_Data != nullptr)
				delete[] m_Data;
		}

		/**
		 * @brief Copy operator
		 * @param buffer The buffer to be copied into this buffer
		 * @return This buffer
		*/
		Buffer& operator=(const Buffer& buffer)
		{
			if (this != &buffer)
			{
				m_Size = buffer.m_Size;
				m_Data = new char[buffer.m_Size];
				memcpy(m_Data, buffer.m_Data, buffer.m_Size);
			}

			return *this;
		}

		/**
		 * @brief Move operator
		 * @param buffer The buffer to be moved into this buffer
		 * @return This buffer
		*/
		Buffer& operator=(Buffer&& buffer) noexcept
		{
			if (this != &buffer)
			{
				if (m_Data != nullptr)
					delete[] m_Data;
				
				m_Data = buffer.m_Data;
				m_Size = buffer.m_Size;

				buffer.m_Data = nullptr;
				buffer.m_Size = 0;
			}

			return *this;
		}

		/**
		 * @brief Copy memory into this buffer
		 * @param data A pointer to the data to copy
		 * @param size The size in bytes of the data to copy
		*/
		inline void load(char* data, size_t size)
		{
			if (size > m_Size)
				throw std::runtime_error("Size is greater than the size of this buffer");

			if (size < m_Size)
				clear();

			memcpy_s(m_Data, m_Size, data, size);
		}

		/**
		 * @brief Zero all memory in this buffer
		*/
		inline void clear() noexcept
		{
			memset(m_Data, 0, m_Size);
		}

		/**
		 * @brief Get the data in this buffer
		 * @return A pointer to the data in this buffer
		*/
		inline char* data() const
		{
			return m_Data;
		}

		/**
		 * @brief Get the size of this buffer
		 * @return The size of this buffer
		*/
		inline size_t size() const
		{
			return m_Size;
		}
	private:
		char* m_Data;
		size_t m_Size;
	};
}
