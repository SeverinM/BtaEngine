#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( binding = 2 ) uniform sampler2D Colormap ;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(Colormap, uv);
}