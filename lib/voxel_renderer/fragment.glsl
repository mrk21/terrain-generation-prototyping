#version 410 core

in vec3 face_color;
out vec3 color;

void main(void) {
    color = face_color;
}
