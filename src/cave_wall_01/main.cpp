#include <iostream>
#include <random>
#include <cmath>
#include <noise/noise.h>
#include <lib/voxel_renderer/voxel_renderer.hpp>
#include <optional>

static const auto PI = boost::math::constants::pi<float>();

struct CaveInfo {
    glm::vec3 p_direction{ 1.0f, 0.0f, 0.0f };
    glm::vec3 s_direction{ 0.0f, 1.0f, 0.0f };
    glm::vec3 w_rotation_axis{ 0.0f, 0.0f, 1.0f };
    glm::vec3 h_rotation_axis{ 0.0f, 1.0f, 0.0f };
};

class CaveWallGenerator {
    const CaveInfo & info;
    uint32_t base_radius;
    std::vector<glm::vec3> base_wall;
    noise::module::Perlin radius_noise;
    std::optional<std::vector<glm::vec3>> prev_wall;

public:
    CaveWallGenerator(int32_t base_seed, const CaveInfo & info_) : info(info_) {
        base_radius = 20u;

        radius_noise.SetSeed(base_seed + 1);
        radius_noise.SetOctaveCount(3);
        radius_noise.SetFrequency(7.0f);

        make_base_wall();
    }

    void generate(
        VoxelRenderer::Vertices & vertices,
        const glm::vec3 & current_position,
        const glm::mat4 & m
    ) {
        using namespace glm;

        std::optional<vec3> prev_wall_point;
        std::vector<vec3> current_wall;
        auto size = base_wall.size();

        for (uint32_t i = 0; i <= size; ++i) {
            auto current_wall_point = vec3(m * vec4(base_wall[i % size], 1.0f));
            current_wall_point *= 1.0f + radius_noise_value(current_wall_point + current_position);
            current_wall_point += current_position;
            current_wall.push_back(current_wall_point);
            vertices.push_back({ current_wall_point.x, current_wall_point.y, current_wall_point.z });

            fill_between_position_and_wall_point(vertices, current_position, current_wall_point);
            fill_between_wall_points(vertices, current_position, current_wall_point, prev_wall_point);
            prev_wall_point = current_wall_point;
        }

        fill_between_walls(vertices, current_position, current_wall);
        prev_wall = current_wall;
    }

    void fill_between_wall_points(
        VoxelRenderer::Vertices & vertices,
        const glm::vec3 & current_position,
        const glm::vec3 & current_wall_point,
        const std::optional<glm::vec3> & prev_wall_point
    ) {
        using namespace glm;
        if (!prev_wall_point) return;

        auto direction = current_wall_point - *prev_wall_point;
        auto distance = length(direction);
        uint32_t distance_i = std::ceil(distance);

        for (uint32_t ti = 1; ti < distance_i; ++ti) {
            auto t = float(ti) / distance_i;
            auto v = lerp(*prev_wall_point, current_wall_point, t);
            vertices.push_back({ v.x, v.y, v.z });

            fill_between_position_and_wall_point(vertices, current_position, v);
        }
    }

    void fill_between_walls(
        VoxelRenderer::Vertices & vertices,
        const glm::vec3 & current_position,
        const std::vector<glm::vec3> & current_wall
    ) {
        using namespace glm;
        if (!prev_wall) return;

        float max_distance = 0.0f;

        for (uint32_t i = 0; i < base_wall.size(); ++i) {
            auto direction = current_wall[i] - (*prev_wall)[i];
            max_distance = std::max(max_distance, length(direction));
        }

        auto max_distance_i = std::ceil(max_distance);
        std::optional<vec3> prev_wall_point;

        for (uint32_t ti = 1; ti < max_distance_i; ++ti) {
            for (uint32_t i = 0; i < base_wall.size(); ++i) {
                auto t = float(ti) / max_distance_i;
                auto current_wall_point = lerp((*prev_wall)[i], current_wall[i], t);
                vertices.push_back({ current_wall_point.x, current_wall_point.y, current_wall_point.z });

                fill_between_wall_points(vertices, current_position, current_wall_point, prev_wall_point);
                prev_wall_point = current_wall_point;
            }
        }
    }

    void fill_between_position_and_wall_point(
        VoxelRenderer::Vertices & vertices,
        const glm::vec3 & current_position,
        const glm::vec3 & current_wall_point
    ) {
    #if 1
        using namespace glm;
        auto direction = current_wall_point - current_position;
        auto distance = length(direction);
        uint32_t distance_i = std::ceil(distance);

        for (uint32_t ti = 0; ti <= distance_i; ++ti) {
            auto t = float(ti) / distance_i;
            auto v = lerp(current_position, current_wall_point, t);
            vertices.push_back({ v.x, v.y, v.z });
        }
    #endif
    }

    float radius_noise_value(const glm::vec3 & p) const {
    #if 1
        using namespace glm;
        static auto f = 255.0f;
        return 0.5f * float(radius_noise.GetValue(p.x / f, p.y / f, p.z / f));
    #else
        return 0.0f;
    #endif
    }

    glm::vec3 lerp(const glm::vec3 & v1, const glm::vec3 & v2, float t) const {
        using namespace glm;
        t = clamp(t, 0.0f, 1.0f);
        return t * (v2 - v1) + v1;
    }

    void make_base_wall() {
        using namespace glm;
        const uint32_t n = 10;

        for (uint32_t i = 0; i < n; ++i) {
            auto rad = float(i) * 2.0f * PI / float(n);
            auto m = rotate(rad, info.p_direction);
            auto v = vec3(m * vec4(float(base_radius) * info.s_direction, 1.0f));
            base_wall.push_back(v);
        }
    }
};

class CaveGenerator {
    int32_t base_seed;
    CaveInfo info;
    CaveWallGenerator wall_generator;

public:
    CaveGenerator(int32_t base_seed_) :
        base_seed(base_seed_),
        info(),
        wall_generator(base_seed, info)
    {}

    VoxelRenderer::Vertices generate() {
        using namespace glm;
        VoxelRenderer::Vertices vertices;

        vec3 prev_position(0.0f, 0.0f, 0.0f);

        {
            auto m = mat4(1.0f);
            make_cave(100u, m, vertices, prev_position);
        }
        {
            auto m = rotate(PI / 4.0f, info.h_rotation_axis);
            make_cave(100u, m, vertices, prev_position);
        }
        {
            auto m = rotate(-PI / 2.0f, info.w_rotation_axis);
            make_cave(100u, m, vertices, prev_position);
        }

        return vertices;
    }

private:
    void make_cave(
        uint32_t cave_length,
        const glm::mat4 & m,
        VoxelRenderer::Vertices & vertices,
        glm::vec3 & prev_position
    ) {
        using namespace glm;

        for (uint32_t i = 0; i < cave_length; ++i) {
            auto current_direction = vec3(m * vec4(info.p_direction, 1.0f));
            auto current_position = current_direction + prev_position;
            wall_generator.generate(vertices, current_position, m);
            prev_position = current_position;
        }
    }
};

int main() {
    try {
        auto window = GLHelpers::init("perlin worms 05");
        VoxelRenderer::Renderer renderer;
        renderer.init(window);

        std::random_device rand_u32;
        auto seed = static_cast<int32_t>(rand_u32());

        CaveGenerator cave(seed);
        auto vertices = cave.generate();
        renderer.render(vertices);
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
        return 1;
    }
    return 0;
}
