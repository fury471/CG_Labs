#version 410

struct ViewProjTransforms
{
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
	ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
	ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;

float calculate_shadow(vec3 world_position){
	// transform fragment to light's clip space
	vec4 light_space = lights[light_index].view_projection * vec4(world_position, 1.0f);
	vec3 proj_coords = (light_space.xyz / light_space.w) * 0.5f + 0.5f;
	float closest_depth = 0.0f;
	float current_depth = proj_coords.z;
	float shawdows = 0.0f;

	for (int i = -1; i <= 1; ++i){
		for (int j = -1; j <= 1; ++j){
			closest_depth = textureOffset(shadow_texture, proj_coords.xy, ivec2(i,j)).r;
			shawdows += (current_depth - 0.00001 > closest_depth)? 1.0f : 0.0f;
		}
	}
	return shawdows / 9.0f;
}

void main()
{
	vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

	// windows coordinates to texture uv
	vec2 uv = gl_FragCoord.xy * inverse_screen_resolution;
	vec3 normal = normalize(texture(normal_texture, uv).xyz * 2.0f - 1.0f);
	float depth = texture(depth_texture, uv).r;

	// NDC to world position
	vec4 ndc = vec4((uv * 2.0f - 1.0f), depth * 2.0f - 1.0f, 1.0f);
	vec4 world_position_homo = camera.view_projection_inverse * ndc;
	vec3 world_position = world_position_homo.xyz / world_position_homo.w;

	vec3 view = normalize(camera_position - world_position);
	vec3 light = normalize(light_position - world_position);
	vec3 reflection = reflect(-light, normal);

	float light_surface_square_dist = max(dot((light_position - world_position), (light_position - world_position)), 0.0001f) * 1.0f;
	float angular_decay = pow(smoothstep(cos(light_angle_falloff), 1.0f, dot(normalize(light_direction), -light)), 10.0f);

	float shadow = calculate_shadow(world_position);

	light_diffuse_contribution  = vec4((1.0f - shadow) * angular_decay * 1.0f/light_surface_square_dist * light_intensity * light_color * max(0.0f, dot(light, normal)), 1.0f);
	light_specular_contribution = vec4((1.0f - shadow) * angular_decay * 1.0f/light_surface_square_dist * light_intensity * light_color * pow(max(0.0f, dot(reflection, view)), 50.0f), 1.0f);
}
