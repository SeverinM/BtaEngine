#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( binding = 0 ) uniform View {
mat4 view;
} uboV;

layout ( binding = 1 ) uniform Projection {
mat4 projection;
} uboP;

layout ( binding = 2 ) uniform Model {
mat4 mModel;
} data;

layout ( location = 0 ) in vec3 inPosition;
layout ( location = 1 ) in vec3 inNormal;
layout ( location = 2 ) in vec2 inUV;
layout ( location = 3 ) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

void main() 
{
    gl_Position = uboP.projection * uboV.view * data.mModel * vec4(inPosition, 1.0);
    fragColor = inColor;
}