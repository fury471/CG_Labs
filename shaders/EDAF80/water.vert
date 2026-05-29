#version 410

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec3 texcoord;

uniform float elapsed_time_s;
uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

out VS_OUT {
	vec3 vertex;
	vec2 uv;
	vec3 normal;
	mat3 TBN;
	vec2 normal_texcoord0;
	vec2 normal_texcoord1;
	vec2 normal_texcoord2;
} vs_out;

struct WaveSample {
	float hight;
	vec2 gradxz;
}wavesample;

WaveSample wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
{
	float params = (position.x * direction.x + position.y * direction.y) * frequency + phase * time;
	float alpha = sin(params) * 0.5 + 0.5;
	wavesample.hight = amplitude * pow(alpha, sharpness);
	float gradx = 0.5 * sharpness * frequency * amplitude * pow(alpha, sharpness-1) * cos(params) * direction.x;
	float gradz = 0.5 * sharpness * frequency * amplitude * pow(alpha, sharpness-1) * cos(params) * direction.y;
	wavesample.gradxz = vec2(gradx, gradz);
	return wavesample;

}

void main()
{
	vec3 displaced_vertex = vertex;
	WaveSample w1 = wave(vertex.xz, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, elapsed_time_s);
	WaveSample w2 = wave(vertex.xz, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, elapsed_time_s);
	displaced_vertex.y += w1.hight;
	displaced_vertex.y += w2.hight;
	vec2 sum_gradxz = w1.gradxz + w2.gradxz;
	vec3 local_n = normalize(vec3(-sum_gradxz.x, 1, -sum_gradxz.y));
	vec3 local_t = normalize(vec3(1, sum_gradxz.x, 0));
	vec3 local_b = normalize(vec3(0, sum_gradxz.y, 1));

	vec3 world_t = normalize(vec3(vertex_model_to_world * vec4(local_t,0.0)));
	vec3 world_b = normalize(vec3(vertex_model_to_world * vec4(local_b,0.0)));
	vec3 world_n = normalize(cross(world_t, world_b));


	vec2 texScale = vec2(8,4);
	float normalTime = mod(elapsed_time_s, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0.0);

	vs_out.normal_texcoord0.xy = texcoord.xz * texScale + normalTime * normalSpeed;
	vs_out.normal_texcoord1.xy = texcoord.xz * texScale * 2 + normalTime * normalSpeed * 4;
	vs_out.normal_texcoord2.xy = texcoord.xz * texScale * 4 + normalTime * normalSpeed * 8;


	vs_out.TBN = mat3(world_t, world_b, world_n);
	vs_out.normal = world_n;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.uv = texcoord.xz;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);
}



