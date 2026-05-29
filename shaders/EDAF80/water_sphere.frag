// water.frag
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
    float facing = 1.0 - max(dot(n, v), 0.0);
    return mix(deep, shallow, facing);
}

void main()
{
    vec3 view = normalize(camera_position - fs_in.vertex);

    vec3 n_tan;
    if (use_normal_mapping == 1) {
        vec3 n0 = texture(normal_map, fs_in.normal_texcoord0).rgb * 2.0 - 1.0;
        vec3 n1 = texture(normal_map, fs_in.normal_texcoord1).rgb * 2.0 - 1.0;
        vec3 n2 = texture(normal_map, fs_in.normal_texcoord2).rgb * 2.0 - 1.0;
        n_tan = normalize(n0 + n1 + n2);
    } else {
        n_tan = vec3(0.0, 0.0, 1.0);
    }

    vec3 world_normal = normalize(fs_in.TBN * n_tan);
    world_normal = faceforward(world_normal, -view, fs_in.normal);

    bool camera_above = gl_FrontFacing;
    float ior_from = camera_above ? 1.0 : 1.33;
    float ior_to   = camera_above ? 1.33 : 1.0;
    float eta      = ior_from / ior_to;

    vec3 reflection_dir = reflect(-view, world_normal);
    vec3 refraction_dir = refract(-view, world_normal, eta);

    vec3 reflection_col = (has_reflection == 1) ? texture(cubemap, reflection_dir).rgb : vec3(0.0);
    vec3 refraction_col = (has_refraction == 1 && dot(refraction_dir, refraction_dir) > 0.0)
                        ? texture(cubemap, refraction_dir).rgb : vec3(0.0);

    float ndv = clamp(dot(world_normal, view), 0.0, 1.0);
    float R0  = pow((ior_from - ior_to) / (ior_from + ior_to), 2.0);
    float fresnel = R0 + (1.0 - R0) * pow(1.0 - ndv, 5.0);

    vec3 color_deep    = vec3(0.0, 0.0, 0.1);
    vec3 color_shallow = vec3(0.0, 0.5, 0.5);
    vec3 water_color   = watercolor(color_shallow, color_deep, world_normal, view);

    vec3 final_col = water_color
                   + reflection_col * fresnel
                   + refraction_col * (1.0 - fresnel);

    frag_color = vec4(final_col, 1.0);
}
