#version 450

layout ( binding = 1 ) uniform samplerCube Colormap ;

layout(location = 0) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(Colormap, fragPosition);
}