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

#include <lib/gl_helpers.hpp>

namespace VoxelRenderer {
    using Vertices = std::vector<std::array<GLfloat, 3>>;

    struct ShaderInfo {
        GLuint id;

        struct {
            GLuint position_location;
        } attribute;

        struct {
            GLuint mvp_location;
            GLuint inv_mvp_location;
            GLuint light_direction_location;
            GLuint camera_position_location;
            GLuint camera_target_location;
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
            info.uniform.inv_mvp_location = glGetUniformLocation(info.id, "inv_mvp");
            info.uniform.light_direction_location = glGetUniformLocation(info.id, "light_direction");
            info.uniform.camera_position_location = glGetUniformLocation(info.id, "camera_position");
            info.uniform.camera_target_location = glGetUniformLocation(info.id, "camera_target");

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

        void bind_params(
            const ShaderInfo & info,
            const glm::mat4 & mvp,
            const glm::vec3 & light_direction,
            const glm::vec3 & camera_position,
            const glm::vec3 & camera_target
        ) {
            glBindVertexArray(vao[0]);
            {
                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(info.attribute.position_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                glm::mat4 inv_mvp = glm::inverse(mvp);
                glUniformMatrix4fv(info.uniform.mvp_location, 1, GL_FALSE, &mvp[0][0]);
                glUniformMatrix4fv(info.uniform.inv_mvp_location, 1, GL_FALSE, &inv_mvp[0][0]);
                glUniformMatrix4fv(info.uniform.light_direction_location, 1, GL_FALSE, &light_direction[0]);
                glUniformMatrix4fv(info.uniform.camera_position_location, 1, GL_FALSE, &camera_position[0]);
                glUniformMatrix4fv(info.uniform.camera_target_location, 1, GL_FALSE, &camera_target[0]);
            }
        }
    };

    class Renderer {
        GLFWwindow * window = nullptr;
        ShaderInfo shader_info;

    public:
        void init(GLFWwindow * window_) {
            window = window_;
            glfwSwapInterval(1);
            glEnable(GL_DEPTH_TEST);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

            ShaderBuilder builder;
            shader_info = builder.build();
        }

        void render(const Vertices & vertices, const glm::vec3 & scale) {
            ShaderDataBinder binder;
            binder.create_buffer(vertices);

            float theta = 0.0f;
            auto animate = [&theta]() {
                static const double pi = boost::math::constants::pi<double>();
                theta += 0.5f * pi / 360.0f;
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
                glm::vec3 light_direction(-1.0f, -2.0f, 2.0f);
                glm::vec3 camera_position(-2.0f * scale.x, -2.0f * scale.y, 2.0f * scale.z);
                glm::vec3 camera_target(0.0f, 0.0f, 0.0f);
                auto view = glm::lookAt(
                    camera_position,
                    camera_target,
                    glm::vec3(0.0f, 0.0f, 1.0f)
                );
                auto model =
                    glm::rotate(theta, glm::vec3(0.0f, 0.0f, 1.0f)) *
                    glm::translate(glm::vec3(-0.5f * scale.x, -0.5f * scale.y, -0.5f * scale.z))
                ;
                auto mvp = projection * view * model;

                binder.bind_params(
                    shader_info,
                    mvp,
                    light_direction,
                    camera_position,
                    camera_target
                );
                glDrawArrays(GL_POINTS, 0, vertices.size() - 1);
                animate();

                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }
    };
}
