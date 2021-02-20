#version 450

// TODO: Comment this out
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Color;

layout (location = 0) out vec3 out_FragColor;

void main()
{
	gl_Position = vec4(in_Position, 1.0);
	out_FragColor = in_Color;
}
