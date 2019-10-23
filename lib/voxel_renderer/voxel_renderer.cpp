#include <glm/gtc/type_ptr.hpp>
#include <lib/voxel_renderer/voxel_renderer.hpp>

namespace VoxelRenderer {
// ShaderBuilder
    ShaderInfo ShaderBuilder::build() {
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
        info.uniform.model_location = glGetUniformLocation(info.id, "model");
        info.uniform.view_location = glGetUniformLocation(info.id, "view");
        info.uniform.projection_location = glGetUniformLocation(info.id, "projection");
        info.uniform.light_direction_location = glGetUniformLocation(info.id, "light_direction");
        info.uniform.camera_position_location = glGetUniformLocation(info.id, "camera_position");
        info.uniform.camera_target_location = glGetUniformLocation(info.id, "camera_target");

        return info;
    }

    GLuint ShaderBuilder::compile(uint32_t type, std::istream & is) {
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

    GLuint ShaderBuilder::compile_from_file(uint32_t type, const std::string & relative_path) {
        auto path = boost::filesystem::path(__FILE__).parent_path().append(relative_path);
        std::ifstream ifs(path.string());
        return compile(type, ifs);
    }

    void ShaderBuilder::validate(uint32_t type, GLuint shader_id) {
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

    const std::map<uint32_t, std::string> ShaderBuilder::SHADER_NAMES{
        { GL_VERTEX_SHADER, "vertex_shader" },
        { GL_GEOMETRY_SHADER, "geometry_shader" },
        { GL_FRAGMENT_SHADER, "fragment_shader" },
    };

// ShaderDataBinder

    void ShaderDataBinder::create_buffer(const Vertices & vertices) {
        glGenBuffers(1, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * vertices.size() * sizeof(GLfloat), &vertices[0][0], GL_STATIC_DRAW);

        glGenVertexArrays(1, vao);
    }

    void ShaderDataBinder::bind_params(
        const ShaderInfo & info,
        const glm::mat4 & model,
        const glm::mat4 & view,
        const glm::mat4 & projection,
        const glm::vec3 & light_direction,
        const glm::vec3 & camera_position,
        const glm::vec3 & camera_target
    ) {
        glBindVertexArray(vao[0]);
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(info.attribute.position_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            glUniformMatrix4fv(info.uniform.model_location, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(info.uniform.view_location, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(info.uniform.projection_location, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3fv(info.uniform.light_direction_location, 1, glm::value_ptr(light_direction));
            glUniform3fv(info.uniform.camera_position_location, 1, glm::value_ptr(camera_position));
            glUniform3fv(info.uniform.camera_target_location, 1, glm::value_ptr(camera_target));
        }
    }

// VerticesOptimizer

    VoxelRenderer::Vertices VerticesOptimizer::optimize(const VoxelRenderer::Vertices & vertices) {
        std::unordered_map<int32_t, std::unordered_map<int32_t, std::unordered_map<int32_t, glm::vec3>>> data;

        for (const auto & v: vertices) {
            auto x = static_cast<int32_t>(std::round(v[0]));
            auto y = static_cast<int32_t>(std::round(v[1]));
            auto z = static_cast<int32_t>(std::round(v[2]));
            data[x][y][z] = glm::vec3(x,y,z);
        }

        VoxelRenderer::Vertices result;
        uint32_t data_size = 0;

        for (const auto & x: data) {
            for (const auto & y: x.second) {
                auto end = y.second.end();
                data_size += y.second.size();
                for (const auto & z: y.second) {
                    if (
                        data[x.first    ][y.first    ].find(z.first - 1) == end ||
                        data[x.first    ][y.first    ].find(z.first + 1) == end ||
                        data[x.first    ][y.first - 1].find(z.first    ) == end ||
                        data[x.first    ][y.first + 1].find(z.first    ) == end ||
                        data[x.first - 1][y.first    ].find(z.first    ) == end ||
                        data[x.first + 1][y.first    ].find(z.first    ) == end
                    ) {
                        result.push_back({ z.second.x, z.second.y, z.second.z });
                    }
                }
            }
        }
        std::cout << "Original vertex size: " << vertices.size() << std::endl;
        std::cout << "Unique vertex size: " << data_size << std::endl;
        std::cout << "Visible vertex size: " << result.size() << std::endl;

        return result;
    }

// Renderer

    void Renderer::init(GLFWwindow * window_) {
        window = window_;
        glfwSwapInterval(1);
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        ShaderBuilder builder;
        shader_info = builder.build();
    }

    void Renderer::render(const Vertices & vertices_) {
        auto vertices = VerticesOptimizer().optimize(vertices_);

        glm::vec3 min( 999999, 999999, 999999);
        glm::vec3 max(-999999,-999999,-999999);
        for (const auto & v: vertices) {
            min.x = std::min(min.x, v[0]);
            min.y = std::min(min.y, v[1]);
            min.z = std::min(min.z, v[2]);

            max.x = std::max(max.x, v[0]);
            max.y = std::max(max.y, v[1]);
            max.z = std::max(max.z, v[2]);
        }
        auto center = (min + max) / 2.0f;

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
                10000.0f
            );
            glm::vec3 light_direction(-0.5f, -1.0f, 0.0f);
            glm::vec3 camera_position(-2.0f * max.x, -2.0f * max.x, 2.0f * max.z);
            glm::vec3 camera_target(0.0f, 0.0f, 0.0f);
            auto view = glm::lookAt(
                camera_position,
                camera_target,
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
            glm::mat4 model(1.0f);
            model *= glm::rotate(theta, glm::vec3(0.0f, 0.0f, 1.0f));
            model *= glm::translate(-center);

            binder.bind_params(
                shader_info,
                model,
                view,
                projection,
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
}
