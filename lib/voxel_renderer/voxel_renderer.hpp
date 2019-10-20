#ifndef VOXEL_RENDERER_HPP
#define VOXEL_RENDERER_HPP

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
            GLuint model_location;
            GLuint view_location;
            GLuint projection_location;
            GLuint light_direction_location;
            GLuint camera_position_location;
            GLuint camera_target_location;
        } uniform;
    };

    class ShaderBuilder {
    public:
        ShaderInfo build();

    private:
        static const std::map<uint32_t, std::string> SHADER_NAMES;

        GLuint compile(uint32_t type, std::istream & is);
        GLuint compile_from_file(uint32_t type, const std::string & relative_path);
        void validate(uint32_t type, GLuint shader_id);
    };

    class ShaderDataBinder {
        GLuint vbo[1];
        GLuint vao[1];

    public:
        void create_buffer(const Vertices & vertices);
        void bind_params(
            const ShaderInfo & info,
            const glm::mat4 & model,
            const glm::mat4 & view,
            const glm::mat4 & projection,
            const glm::vec3 & light_direction,
            const glm::vec3 & camera_position,
            const glm::vec3 & camera_target
        );
    };

    class Renderer {
        GLFWwindow * window = nullptr;
        ShaderInfo shader_info;

    public:
        void init(GLFWwindow * window_);
        void render(const Vertices & vertices, const glm::vec3 & scale);
    };
}

#endif
