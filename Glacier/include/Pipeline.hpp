#pragma once

#include "Shader.hpp"

#include <vector>

namespace glacier
{
	struct Pipeline
	{
		std::vector<Shader*> shaders;
	};
}
