#include <iostream>
#include <memory>

#include <glacier.hpp>
#include <SandboxApp.hpp>

int main()
{
	std::shared_ptr<SandboxApp> app;

	try
	{
		app = std::make_shared<SandboxApp>();
		app->run();
	}
	catch (const std::exception& e)
	{
		glacier::Application::s_Logger->error("{}", e.what());
		
	}
	
	return 0;
}
