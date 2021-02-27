#pragma once

#include "Buffer.hpp"
#include "common.hpp"

#include <string_view>

namespace glacier
{
	/**
	 * @brief Class representing a file on the user's file system
	*/
	class File
	{
	public:
		/**
		 * @brief Create an instance of File
		 * @param path Path to the file
		*/
		GLACIER_API File(std::string_view path);

		/**
		 * @brief Destroy this instance
		*/
		GLACIER_API ~File();

		/**
		 * @brief Read this file to a stack allocated buffer
		 * @return A buffer allocated on the stack
		*/
		GLACIER_API Buffer read() const;

		/**
		 * @brief Read this file to a heap allocated buffer
		 * @return A buffer allocated on the heap
		*/
		GLACIER_API Buffer* read_ptr() const;

		/**
		 * @brief Set the base directory of all future file instances. Prepended to the file path.
		 * @param directory Base directory
		*/
		inline static void setBaseDirectory(std::string_view directory)
		{
			s_BaseDirectory = directory;
		}
	private:
		std::string m_Path;
		GLACIER_API static std::string s_BaseDirectory;
	};
}
