#include "core/common.hpp"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> glacier::g_Logger = spdlog::stdout_color_mt("console");
