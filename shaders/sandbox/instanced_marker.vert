#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_instance_offset;
layout(location = 2) in vec3 in_instance_colour;

uniform mat4 u_world_to_clip;
uniform float u_marker_scale;

out vec3 v_colour;

void main()
{
	v_colour = in_instance_colour;
	vec3 world_position = in_instance_offset + in_position * u_marker_scale;
	gl_Position = u_world_to_clip * vec4(world_position, 1.0);
}
