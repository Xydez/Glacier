#pragma once

#include <glacier.hpp>

glacier::ApplicationInfo generateApplicationInfo()
{
	glacier::WindowInfo windowInfo;
	windowInfo.title = "SandboxApp";
	windowInfo.width = 800;
	windowInfo.height = 600;
	windowInfo.resizable = true;
	windowInfo.vsync = true;

	glacier::ApplicationInfo info = { "SandboxApp", 0, 1, 0, windowInfo };

	return info;
}

class SandboxApp : public glacier::Application
{
public:
	SandboxApp()
		: Application(generateApplicationInfo())
	{}

	~SandboxApp()
	{}
};
