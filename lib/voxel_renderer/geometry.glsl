#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 36) out;
out vec3 face_normal;

uniform mat4 mvp;

void main(void) {
    vec3[8] vertices = vec3[](
        vec3( 1.0, 1.0, 1.0 ),
        vec3( 0.0, 1.0, 1.0 ),
        vec3( 0.0, 0.0, 1.0 ),
        vec3( 1.0, 0.0, 1.0 ),
        vec3( 1.0, 0.0, 0.0 ),
        vec3( 1.0, 1.0, 0.0 ),
        vec3( 0.0, 1.0, 0.0 ),
        vec3( 0.0, 0.0, 0.0 )
    );
    int[36] faces = int[](
        0,1,2, 0,2,3,
        0,3,4, 0,4,5,
        0,5,6, 0,6,1,
        1,6,7, 1,7,2,
        7,4,3, 7,3,2,
        4,7,6, 4,6,5
    );

    for (int i=0; i<6; i++) {
        vec3 v0 = vertices[faces[i*6 + 0]];
        vec3 v1 = vertices[faces[i*6 + 1]];
        vec3 v2 = vertices[faces[i*6 + 2]];
        vec3 vv1 = v1 - v0;
        vec3 vv2 = v2 - v1;
        face_normal = normalize(cross(vv1, vv2));

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
}
