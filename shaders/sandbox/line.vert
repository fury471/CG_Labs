#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;

uniform mat4 u_world_to_clip;

out vec3 v_colour;

void main()
{
	v_colour = in_colour;
	gl_Position = u_world_to_clip * vec4(in_position, 1.0);
}
