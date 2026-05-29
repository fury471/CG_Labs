#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;


uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 uv;
	mat3 TBN;
} vs_out;


void main()
{
	vec3 world_vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));
	vec3 world_normal = normalize(vec3(normal_model_to_world * vec4(normal, 0.0)));
	vec3 world_tangent = normalize(vec3(vertex_model_to_world * vec4(tangent, 0.0)));
	vec3 world_binormal = normalize(vec3(vertex_model_to_world * vec4(binormal,0.0)));

	// ensure the tangent, bitangent and normal form an orthonormal basis
	world_tangent = normalize(world_tangent - dot(world_tangent, world_normal) * world_normal);
	world_binormal = normalize(cross(world_normal, world_tangent));

	vs_out.vertex = world_vertex;
	vs_out.normal = world_normal;
	vs_out.TBN = mat3(world_tangent, world_binormal, world_normal);
	vs_out.uv = texcoord.xy;

	gl_Position = vertex_world_to_clip * vec4(world_vertex, 1.0);
}



