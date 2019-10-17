#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <string>
#include <functional>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/math/constants/constants.hpp>

#include <noise/noise.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#define GLFW_INCLUDE_NONE
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

struct ShaderInfo {
    GLuint id;

    struct {
        GLuint position_location;
    } attribute;

    struct {
        GLuint mvp_location;
    } uniform;
};

class ShaderBuilder {
public:
    ShaderInfo build() {
        ShaderInfo info;

        GLuint v_shader_id = compile_from_file(GL_VERTEX_SHADER, "vertex.glsl");
        GLuint g_shader_id = compile_from_file(GL_GEOMETRY_SHADER, "geometry.glsl");
        GLuint f_shader_id = compile_from_file(GL_FRAGMENT_SHADER, "fragment.glsl");

        info.id = glCreateProgram();
        glAttachShader(info.id, v_shader_id);
        glAttachShader(info.id, g_shader_id);
        glAttachShader(info.id, f_shader_id);
        glLinkProgram(info.id);

        info.attribute.position_location = glGetAttribLocation(info.id, "position");
        info.uniform.mvp_location = glGetUniformLocation(info.id, "mvp");

        return info;
    }

private:
    static const std::map<uint32_t, std::string> SHADER_NAMES;

    GLuint compile(uint32_t type, std::istream & is) {
        std::string source;
        std::copy(
            std::istreambuf_iterator<char>(is),
            std::istreambuf_iterator<char>(),
            std::insert_iterator<std::string>(source, source.end())
        );

        GLuint shader_id = glCreateShader(type);
        const char * s = source.c_str();
        glShaderSource(shader_id, 1, &s, nullptr);
        glCompileShader(shader_id);
        validate(type, shader_id);

        return shader_id;
    }

    GLuint compile_from_file(uint32_t type, const std::string & relative_path) {
        auto path = boost::filesystem::path(__FILE__).parent_path().append(relative_path);
        std::ifstream ifs(path.string());
        return compile(type, ifs);
    }

    void validate(uint32_t type, GLuint shader_id) {
        const std::string & shader_name = SHADER_NAMES.at(type);
        GLint is_compiled = GL_FALSE;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_compiled);

        if (is_compiled == GL_FALSE) {
            GLint length = 0;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

            std::string log;
            log.resize(length);
            glGetShaderInfoLog(shader_id, length, &length, &log[0]);
            std::cerr << boost::format("%s: %s") % shader_name % log << std::endl;

            glDeleteShader(shader_id);
            throw (boost::format("%s is invalid") % shader_name).str();
        }
    }
};
const std::map<uint32_t, std::string> ShaderBuilder::SHADER_NAMES{
    { GL_VERTEX_SHADER, "vertex_shader" },
    { GL_GEOMETRY_SHADER, "geometry_shader" },
    { GL_FRAGMENT_SHADER, "fragment_shader" },
};

class ShaderDataBinder {
    GLuint vbo[1];
    GLuint vao[1];

public:
    void create_buffer(const Vertices & vertices) {
        glGenBuffers(1, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * vertices.size() * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

        glGenVertexArrays(1, vao);
    }

    void bind_params(const ShaderInfo & info, const glm::mat4 & mvp) {
        glBindVertexArray(vao[0]);
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(info.attribute.position_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glUniformMatrix4fv(info.uniform.mvp_location, 1, GL_FALSE, &mvp[0][0]);
        }
    }
};

GLFWwindow * init(const std::string & title) {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(640, 480, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    std::cout << "## OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    return window;
}

int main() {
    try {
        auto window = init("perlin noise 3d");

        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        ShaderBuilder builder;
        auto shader_info = builder.build();

        NoiseGenerator noise(100, 100, 100);
        Vertices vertices = noise.generate();

        ShaderDataBinder binder;
        binder.create_buffer(vertices);

        float theta = 0.0f;
        auto animate = [&theta]() {
            static const double pi = boost::math::constants::pi<double>();
            theta += 1.0f * pi / 360.0f;
        };

        while (glfwWindowShouldClose(window) == GL_FALSE) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shader_info.id);

            int ww, wh;
            glfwGetFramebufferSize(window, &ww, &wh);

            auto projection = glm::perspective(
                glm::radians(30.0f),
                1.0f * ww / wh,
                0.1f,
                2000.0f
            );
            auto view = glm::lookAt(
                glm::vec3(
                    -2.0f * noise.x_length,
                    -2.0f * noise.y_length,
                     2.0f * noise.z_length
                ),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
            auto model =
                glm::rotate(
                    theta,
                    glm::vec3(0.0f, 0.0f, 1.0f)
                )
                *
                glm::translate(glm::vec3(
                    -0.5 * noise.x_length,
                    -0.5 * noise.y_length,
                    -0.5 * noise.z_length
                ))
            ;
            auto mvp = projection * view * model;

            binder.bind_params(shader_info, mvp);
            glDrawArrays(GL_POINTS, 0, vertices.size() - 1);
            animate();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        return EXIT_SUCCESS;
    } catch (std::string str) {
        std::cerr << str << std::endl;
    }
}
