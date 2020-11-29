#version 450

layout ( location = 0 ) in vec2 position;
layout ( location = 1 ) in vec2 tex;

layout ( location = 0 ) out vec2 TexCoords;

layout ( binding = 1 ) uniform VP {
	mat4 view;
	mat4 projection;
} ubo;

layout ( binding = 2 ) uniform M
{
	mat4 model;
} data;

void main()
{
	gl_Position = ubo.projection * ubo.view * data.model * vec4(position.xy, 0.0,1.0);
	TexCoords = tex;
}