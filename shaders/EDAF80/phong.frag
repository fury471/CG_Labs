#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;
uniform int use_normal_mapping;
uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;

uniform float time;


in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 uv;
	mat3 TBN;
} fs_in;

out vec4 frag_color;


void main()
{
	vec3 world_normal = normalize(fs_in.normal);
	if (use_normal_mapping == 1) {
		vec3 n = texture(normal_map,fs_in.uv).xyz * 2.0 - 1.0;
		world_normal= normalize(fs_in.TBN * n);
	}

	vec3 vec_light = normalize(light_position - fs_in.vertex);
	vec3 vec_view = normalize(camera_position - fs_in.vertex);
	vec3 vec_half = normalize(vec_light + vec_view);


	vec3 kd_tex = diffuse_colour * texture(diffuse_map, fs_in.uv).rgb;
	vec3 ks_tex = specular_colour * texture(specular_map, fs_in.uv).rgb;


	vec3 diffuse = kd_tex * max(dot(world_normal, vec_light),0.0);
	vec3 specular = ks_tex * pow(max(dot(world_normal, vec_half),0.0), shininess_value);
	frag_color = vec4(ambient_colour + diffuse + specular, 1.0);


}
