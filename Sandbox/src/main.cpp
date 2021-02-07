#include <iostream>
#include <memory>

#include <glacier.hpp>
#include <SandboxApp.hpp>

int main()
{
	glacier::g_Logger->set_level(spdlog::level::debug);

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
