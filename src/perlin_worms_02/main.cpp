#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <array>
#include <algorithm>

#include <noise/noise.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

double clamp(double v, double l, double u) {
    if (v < l) return l;
    if (v > u) return u;
    return v;
}

double threshold(double v, double t, double l, double u) {
    return v < t ? l : u;
}

using Vertices = std::vector<std::array<GLfloat, 3>>;

class NoiseGenerator {
public:
    uint32_t x_length;
    uint32_t y_length;
    uint32_t z_length;

    NoiseGenerator(uint32_t x_length_, uint32_t y_length_, uint32_t z_length_) {
        x_length = x_length_;
        y_length = y_length_;
        z_length = z_length_;
    }

    Vertices generate() {
        Vertices vertices;
        noise::module::Perlin perlin;
        std::random_device rand_u32;
        perlin.SetSeed(static_cast<int32_t>(rand_u32()));
        perlin.SetOctaveCount(6);
        perlin.SetFrequency(2.0);

        std::cout << "seed: " << perlin.GetSeed() << std::endl;

        for (uint32_t x = 0; x < x_length; ++x) {
            for (uint32_t y = 0; y < y_length; ++y) {
                for (uint32_t z = 0; z < z_length; ++z) {
                    float v = perlin.GetValue(
                        1.0 * x / x_length,
                        1.0 * y / y_length,
                        1.0 * z / z_length
                    );
                    v = clamp(v, -1.0, 1.0);
                    v = threshold(std::abs(v), 0.1, 0.0, 1.0);
                    if (v == 0.0) {
                        vertices.push_back({ static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z) });
                    }
                }
            }
        }

        return vertices;
    }
};

class Cube {
    static constexpr GLdouble VERTEX[][3] = {
        { 0.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0 },
        { 1.0, 1.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 1.0 },
        { 1.0, 1.0, 1.0 },
        { 0.0, 1.0, 1.0 }
    };

    static constexpr int FACE[][4] = {
        { 0, 1, 2, 3 },
        { 1, 5, 6, 2 },
        { 5, 4, 7, 6 },
        { 4, 0, 3, 7 },
        { 4, 5, 1, 0 },
        { 3, 2, 6, 7 }
    };

    static constexpr GLdouble NORMAL[][3] = {
        { 0.0, 0.0,-1.0 },
        { 1.0, 0.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        {-1.0, 0.0, 0.0 },
        { 0.0,-1.0, 0.0 },
        { 0.0, 1.0, 0.0 }
    };

    static constexpr GLfloat MATERIAL[] = { 0.8, 0.2, 0.2, 1.0 };

public:
    void draw() {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, MATERIAL);
        glBegin(GL_QUADS);
        {
            for (uint32_t j = 0; j < 6; ++j) {
                glNormal3dv(NORMAL[j]);
                for (uint32_t i = 0; i < 4; ++i) {
                    glVertex3dv(VERTEX[FACE[j][i]]);
                }
            }
        }
        glEnd();
    }
};

class VoxelMap {
public:
    void draw(const Vertices & vertices) {
        Cube cube;

        std::for_each(vertices.begin(), vertices.end(), [&cube](const auto & v){
            glPushMatrix();
            {
                glTranslated(v[0], v[1], v[2]);
                cube.draw();
            }
            glPopMatrix();
        });
    }
};

int main() {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    GLFWwindow * window = glfwCreateWindow(640, 480, "perlin worms 01", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    NoiseGenerator noise(200, 200, 200);
    Vertices vertices = noise.generate();
    VoxelMap voxel_map;

    const GLfloat light_color[] = { 0.2f, 0.2f, 0.8f, 1.0f };
    const GLfloat light0_pos[] = {
        -0.1f * noise.x_length,
        -0.1f * noise.y_length,
        1.0f * noise.z_length,
        1.0
    };
    const GLfloat light1_pos[] = {
        5.0f, 3.0f, 0.0f, 1.0f
    };

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glCullFace(GL_FRONT);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_color);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_color);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    while (glfwWindowShouldClose(window) == GL_FALSE) {
        // init
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // viewport
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        gluPerspective(30.0, (double)w / (double)h, 1.0, 2000.0);

        glTranslated(0.0, -1.0 * noise.y_length * (3.0 / 4.0), -1.0 * noise.z_length / 2.0);
        gluLookAt(
            -2.0 * noise.x_length, -2.0 * noise.y_length, 2.0 * noise.z_length,
            0.0, 0.0, 0.0,
            0.0, 0.0, 1.0
        );

        GLfloat m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, m);

        // lighting
        glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
        glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);

        // draw
        voxel_map.draw(vertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return 0;
}
