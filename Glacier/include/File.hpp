#pragma once

#include "Buffer.hpp"
#include "common.hpp"

#include <string_view>

namespace glacier
{
	class File
	{
	public:
		GLACIER_API File(std::string_view path);
		GLACIER_API ~File();

		GLACIER_API Buffer read() const;

		inline static void setBaseDirectory(std::string_view directory)
		{
			s_BaseDirectory = directory;
		}
	private:
		std::string m_Path;
		GLACIER_API static std::string s_BaseDirectory;
	};
}
