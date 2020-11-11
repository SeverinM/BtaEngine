#version 450

layout ( location = 0 ) in vec2 TexCoords;
layout ( location = 0 ) out vec4 color;

layout ( binding = 0 ) uniform sampler2D Colormap ;

void main()
{
	vec4 colorSampled = texture(Colormap, vec2( TexCoords.x, 1 - TexCoords.y ));
	color = vec4( 1 - colorSampled.r,1 - colorSampled.r, 1 - colorSampled.r, colorSampled.r);
	
	if ( colorSampled.r == 0)
		color.a = 0;
}