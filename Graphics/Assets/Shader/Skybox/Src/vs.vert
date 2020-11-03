#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( binding = 0 ) uniform UniformBufferObject {
    mat4 model;
    mat4 proj;
} ubo;

layout ( location = 0 ) in vec3 inPosition;

layout ( location = 0 ) out vec3 outPosition;

void main() 
{
	outPosition = inPosition;
	outPosition.xy *= -1.0;
    gl_Position = ubo.proj * ubo.model * vec4(inPosition, 1.0);
}