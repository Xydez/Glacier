#version 450

// TODO: Comment this out
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

void main()
{
	if (int(gl_FragCoord.x) % 10 == 0 || int(gl_FragCoord.y) % 10 == 0)
	{
		outColor = vec4(fragColor, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
