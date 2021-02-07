#pragma once

#include "Shader.hpp"

#include <vector>

namespace glacier
{
	struct PipelineInfo
	{
		std::vector<Shader*> shaders;
	};
}
