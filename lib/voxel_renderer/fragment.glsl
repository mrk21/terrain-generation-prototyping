#version 410 core

in vec3 face_normal;
out vec4 color;

uniform mat4 inv_mvp;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_target;

void main(void) {
    // lighting
    vec4 ambient_color = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 face_color = vec4(0.5, 0.5, 0.5, 1.0);

    vec3 camera_direction = (-1.0 * camera_position - camera_target);
    vec3 inv_eye = normalize(inv_mvp * vec4(camera_direction, 1.0)).xyz;
    vec3 inv_light = normalize(inv_mvp * vec4(light_direction, 1.0)).xyz;
    vec3 half_vector = normalize(inv_light + inv_eye);

    float diffuse = clamp(dot(inv_light, face_normal), 0.1, 1.0);
    float specular = pow(clamp(dot(half_vector, face_normal), 0.0, 1.0), 5.0);
    vec3 dest = face_color.rgb * diffuse + specular + ambient_color.rgb;

    color = vec4(dest, face_color.a);
}
