#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 18) out;
out vec3 face_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_target;

const vec3[8] vertices = vec3[](
    vec3( 1.0, 1.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ),
    vec3( 0.0, 0.0, 1.0 ),
    vec3( 1.0, 0.0, 1.0 ),
    vec3( 1.0, 0.0, 0.0 ),
    vec3( 1.0, 1.0, 0.0 ),
    vec3( 0.0, 1.0, 0.0 ),
    vec3( 0.0, 0.0, 0.0 )
);
const int[36] faces = int[](
    0,1,2, 0,2,3,
    0,3,4, 0,4,5,
    0,5,6, 0,6,1,
    1,6,7, 1,7,2,
    7,4,3, 7,3,2,
    4,7,6, 4,6,5
);

mat4 mvp = projection * view * model;
mat3 transpose_inverse_m = mat3(transpose(inverse(model)));

const vec4 ambient_color = vec4(0.1, 0.1, 0.1, 1.0);
const vec4 face_color_base = vec4(0.5, 0.5, 0.5, 1.0);

vec3 camera_direction = normalize(-1.0 * camera_position - camera_target);
vec3 half_vector = normalize(light_direction + camera_direction);

void display_face(int i) {
    for (int j=0; j<3; j++) {
        gl_Position = mvp * (gl_in[0].gl_Position + vec4(vertices[faces[j + i*6]], 0.0));
        EmitVertex();
    }
    EndPrimitive();

    for (int j=3; j<6; j++) {
        gl_Position = mvp * (gl_in[0].gl_Position + vec4(vertices[faces[j + i*6]], 0.0));
        EmitVertex();
    }
    EndPrimitive();
}

void main(void) {
    for (int i=0; i<6; i++) {
        vec3 v0 = vertices[faces[i*6 + 0]];
        vec3 v1 = vertices[faces[i*6 + 1]];
        vec3 v2 = vertices[faces[i*6 + 2]];
        vec3 vv1 = v1 - v0;
        vec3 vv2 = v2 - v0;
        vec3 face_normal = normalize(transpose_inverse_m * cross(vv1, vv2));

        float face_diffuse = max(0, dot(light_direction, face_normal));
        float face_specular = pow(max(0, dot(half_vector, face_normal)), 8.0);
        face_color = face_color_base.rgb * face_diffuse + face_specular + ambient_color.rgb;

        if (dot(face_normal, camera_direction) <= 0.0) {
            display_face(i);
        }
    }
}
