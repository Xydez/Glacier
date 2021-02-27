#pragma once

#include <spdlog/fwd.h>
#include <memory>

#ifdef GLACIER_BUILD_DLL
#ifdef GLACIER_BUILD
#define GLACIER_API __declspec(dllexport)
#else
#define GLACIER_API __declspec(dllimport)
#endif // GLACIER_BUILD
#else
#define GLACIER_API
#endif // GLACIER_BUILD_DLL

namespace glacier
{
	/**
	 * @brief The global logger
	*/
	extern GLACIER_API std::shared_ptr<spdlog::logger> g_Logger;
}
