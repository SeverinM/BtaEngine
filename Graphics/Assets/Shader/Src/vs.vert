#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( binding = 0 ) uniform ViewProjection {
    mat4 view;
    mat4 proj;
} ubo;

layout ( binding = 1 ) buffer Models
{
	mat4[] mModel;
} data;

layout ( location = 0 ) in vec3 inPosition;
layout ( location = 1 ) in vec2 inColor;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragColor;

void main() 
{
    gl_Position = ubo.proj * ubo.view * data.mModel[gl_InstanceIndex] * vec4(inPosition, 1.0);
	fragPosition = inPosition;
    fragColor = inColor;
}