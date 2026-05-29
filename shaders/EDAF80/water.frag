#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform sampler2D normal_map;
uniform samplerCube cubemap;
uniform int use_normal_mapping;
uniform int has_reflection;
uniform int has_refraction;


in VS_OUT {
	vec3 vertex;
	vec2 uv;
	vec3 normal;
	mat3 TBN;
	vec2 normal_texcoord0;
	vec2 normal_texcoord1;
	vec2 normal_texcoord2;
} fs_in;

out vec4 frag_color;

vec3 watercolor(vec3 shallow, vec3 deep, vec3 normal, vec3 view)
{
	vec3 n = normalize(normal);
	vec3 v = normalize(view);
	float facing = 1.0 - max(dot(n,v), 0.0);
	return vec3(mix(deep, shallow, facing));
}

void main()
{
	
	vec3 norm = normalize(fs_in.normal);
	vec3 reflection = vec3(0.0, 0.0, 0.0);
	vec3 refraction = vec3(0.0, 0.0, 0.0);
	vec3 view = normalize(camera_position - fs_in.vertex);
	vec3 light = normalize(light_position - fs_in.vertex);

	if(use_normal_mapping == 1){
		vec3 n_tan = texture(normal_map,fs_in.normal_texcoord0).xyz * 2.0 - 1.0;
		n_tan += texture(normal_map,fs_in.normal_texcoord1).xyz * 2.0 - 1.0;
		n_tan += texture(normal_map,fs_in.normal_texcoord2).xyz * 2.0 - 1.0;
		norm= normalize(fs_in.TBN * normalize(n_tan));
	}

	bool camera_above = dot(norm,view) > 0.0;
	norm = gl_FrontFacing?norm: -norm;

	if(has_reflection == 1){
		reflection = reflect(-view, norm);
		reflection = texture(cubemap, reflection).xyz;
	}
	
	vec3 color_deep = vec3(0.0, 0.0, 0.1);
    vec3 color_shallow = vec3(0.0, 0.5, 0.5);
	vec3 water_color = watercolor(color_shallow, color_deep, norm, view);


	float n_air = 1.0;
	float n_water = 1.33;
	float n1 = gl_FrontFacing? n_water:n_air;
	float n2 = gl_FrontFacing? n_air:n_water;
	float eta = n1/n2;
	float ndv = clamp(dot(norm, view),0.0, 1.0);
	float R0 = pow((n1-n2)/(n1+n2),2);
	float fresnel = R0 + (1.0 - R0) * pow(1.0-ndv, 5.0);

	if(has_refraction == 1){
		refraction = refract(-view, norm, eta);
		refraction = texture(cubemap, refraction).xyz * (1 - fresnel);
		reflection *= fresnel;
	}

	frag_color = vec4(water_color + reflection  + refraction, 1.0);
}
