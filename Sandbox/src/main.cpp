#include <iostream>
#include <memory>

#include <glacier.hpp>
#include <SandboxApp.hpp>

int main(int argc, char** argv)
{
	/* Parse command-line arguments */
	int resourceDirectoryArgumentIndex = -1;

	for (unsigned int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--resource-dir") == 0)
		{
			if (argc > i + 1)
			{
				i++;

				glacier::File::setBaseDirectory(argv[i]);

				continue;
			}
			else
			{
				glacier::g_Logger->error("Not enough arguments");
				return -1;
			}
		}
		else if (strcmp(argv[i], "--log-level") == 0)
		{
			if (argc > i + 1)
			{
				i++;

				if (strcmp(argv[i], "none") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::off);
				}
				else if (strcmp(argv[i], "trace") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::trace);
				}
				else if (strcmp(argv[i], "debug") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::debug);
				}
				else if (strcmp(argv[i], "info") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::info);
				}
				else if (strcmp(argv[i], "warning") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::warn);
				}
				else if (strcmp(argv[i], "error") == 0)
				{
					glacier::g_Logger->set_level(spdlog::level::err);
				}
				else
				{
					glacier::g_Logger->error("Invalid log level");
					return -1;
				}

				continue;
			}
			else
			{
				glacier::g_Logger->error("Not enough arguments");
				return -1;
			}
		}
		else
		{
			glacier::g_Logger->error("Invalid command-line arguments");
			return -1;
		}
	}
	/* ---------------------------- */

	glacier::g_Logger->set_level(spdlog::level::debug);

	if (resourceDirectoryArgumentIndex >= 0)
		glacier::g_Logger->debug("Using resource directory {}", argv[resourceDirectoryArgumentIndex]);

	std::shared_ptr<SandboxApp> app;

	try
	{
		app = std::make_shared<SandboxApp>();
		app->run();
	}
	catch (const std::exception& e)
	{
		glacier::g_Logger->error("{}", e.what());
	}
	
	return 0;
}
