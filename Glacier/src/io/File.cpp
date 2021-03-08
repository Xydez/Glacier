#include "io/File.hpp"

#include <fstream>
#include <spdlog/fmt/fmt.h>

std::string glacier::File::s_BaseDirectory = ".";

glacier::File::File(std::string_view path)
	: m_Path(s_BaseDirectory + "/" + path.data()) // std::string(s_BaseDirectory).append(path)
{
}

glacier::File::~File()
{
}

glacier::Buffer glacier::File::read() const
{
	std::ifstream stream(m_Path.data(), std::ios::ate | std::ios::binary);

	if (!stream)
	{
		throw std::runtime_error(fmt::format("Failed to read file {}", m_Path));
	}

	size_t size = stream.tellg();
	stream.seekg(0);

	Buffer buffer(size);
	stream.read(buffer.data(), size);

	stream.close();

	return buffer;
}

GLACIER_API glacier::Buffer* glacier::File::read_ptr() const
{
	std::ifstream stream(m_Path.data(), std::ios::ate | std::ios::binary);

	if (!stream)
	{
		throw std::runtime_error(fmt::format("Failed to read file {}", m_Path));
	}

	size_t size = stream.tellg();
	stream.seekg(0);

	Buffer* buffer = new Buffer(size);
	stream.read(buffer->data(), size);

	stream.close();

	return buffer;
}
