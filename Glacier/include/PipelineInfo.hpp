#pragma once

#include "Shader.hpp"
#include "VertexBuffer.hpp"

#include <vector>

namespace glacier
{
	struct PipelineInfo
	{
		std::vector<Shader*> shaders;
		VertexBuffer* vertexBuffer;
		size_t vertexCount;
	};
}
