#version 410 core

in vec3 face_normal;
out vec4 color;

uniform mat4 model;

uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_target;

void main(void) {
    // lighting
    vec4 ambient_color = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 face_color = vec4(0.5, 0.5, 0.5, 1.0);

    vec3 face_normal2 = normalize((model * vec4(face_normal, 0.0)).xyz);
    vec3 camera_direction = normalize(-1.0 * camera_position - camera_target);
    vec3 camera_direction2 = normalize(camera_direction);
    vec3 light_direction2 = normalize(light_direction);
    vec3 half_vector = normalize(light_direction2 + camera_direction2);

    float diffuse = clamp(dot(light_direction2, face_normal2), 0.1, 1.0);
    float specular = pow(clamp(dot(half_vector, face_normal2), 0.0, 1.0), 8.0);
    vec3 dest = face_color.rgb * diffuse + specular + ambient_color.rgb;

    color = vec4(dest, face_color.a);
}
