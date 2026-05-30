#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_colour;

uniform mat4 u_world_to_clip;
uniform float u_point_size;
uniform float u_colour_mode;
uniform vec3 u_solid_colour;

out vec3 v_colour;

void main()
{
	vec3 height_colour = mix(vec3(0.15, 0.45, 1.0), vec3(1.0, 0.9, 0.15), clamp(in_position.y * 0.5, 0.0, 1.0));
	if (u_colour_mode < 0.5)
		v_colour = in_colour;
	else if (u_colour_mode < 1.5)
		v_colour = height_colour;
	else
		v_colour = u_solid_colour;
	gl_Position = u_world_to_clip * vec4(in_position, 1.0);
	gl_PointSize = u_point_size;
}
