#include <iostream>
#include <memory>

#include <glacier.hpp>
#include <SandboxApp.hpp>

int main()
{
	std::shared_ptr<SandboxApp> app = std::make_shared<SandboxApp>();

	try
	{
		app->run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	return 0;
}
