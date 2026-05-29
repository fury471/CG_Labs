// water.vert
#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
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
} wavesample;

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
    vec3 P_world = (vertex_model_to_world * vec4(vertex, 1.0)).xyz;
    vec3 N_world = normalize((normal_model_to_world * vec4(normalize(normal), 0.0)).xyz);

    vec3 helper  = (abs(N_world.y) < 0.999) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 T_world = normalize(cross(helper, N_world));
    vec3 B_world = normalize(cross(N_world, T_world));

    vec2 p = vec2(dot(P_world, T_world), dot(P_world, B_world));

    WaveSample w1 = wave(p, normalize(vec2(-1.0,  0.0)), 1.0, 0.2, 0.5, 2.0, elapsed_time_s);
    WaveSample w2 = wave(p, normalize(vec2(-0.7,  0.7)), 0.5, 0.4, 1.3, 2.0, elapsed_time_s);

    float H = w1.hight + w2.hight;
    vec2  g = w1.gradxz + w2.gradxz;

    P_world += N_world * H;

    vec3 N_shade = normalize(N_world - g.x * T_world - g.y * B_world);

    T_world = normalize(T_world - N_shade * dot(N_shade, T_world));
    B_world = normalize(cross(N_shade, T_world));

    float t = elapsed_time_s;
    vec2 texScale = vec2(6.0);
    vec2 uv_tb = vec2(dot(P_world, T_world), dot(P_world, B_world));
    vs_out.normal_texcoord0 = uv_tb * texScale         + t * vec2( 0.06,  0.03);
    vs_out.normal_texcoord1 = uv_tb * (texScale * 1.7) + t * vec2(-0.04,  0.05);
    vs_out.normal_texcoord2 = uv_tb * (texScale * 3.0) + t * vec2( 0.02, -0.06);

    vs_out.TBN    = mat3(T_world, B_world, N_shade);
    vs_out.normal = N_shade;
    vs_out.vertex = P_world;
    vs_out.uv     = texcoord.xy;

    gl_Position = vertex_world_to_clip * vec4(P_world, 1.0);
}
