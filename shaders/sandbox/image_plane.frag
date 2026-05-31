#version 460 core

in vec2 v_texcoord;

layout(location = 0) out vec4 out_colour;

uniform sampler2D u_image;

void main()
{
	out_colour = vec4(texture(u_image, v_texcoord).rgb, 1.0);
}
