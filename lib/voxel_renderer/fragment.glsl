#version 410 core

in vec3 face_normal;
out vec4 color;

uniform mat4 model;
uniform mat4 view;

uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_target;

void main(void) {
    // lighting
    vec4 ambient_color = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 face_color = vec4(0.5, 0.5, 0.5, 1.0);

    mat4 mv = view * model;
    mat4 inv_mv = inverse(mv);

    vec3 camera_direction = normalize(-1.0 * camera_position - camera_target);
    vec3 camera_direction2 = normalize((inv_mv * vec4(camera_direction, 1.0)).xyz);
    vec3 light_direction2 = normalize((inv_mv * vec4(light_direction, 1.0)).xyz);
    vec3 half_vector = normalize(light_direction2 + camera_direction2);

    float diffuse = clamp(dot(light_direction2, face_normal), 0.1, 1.0);
    float specular = pow(clamp(dot(half_vector, face_normal), 0.0, 1.0), 8.0);
    vec3 dest = face_color.rgb * diffuse + specular + ambient_color.rgb;

    color = vec4(dest, face_color.a);
}
