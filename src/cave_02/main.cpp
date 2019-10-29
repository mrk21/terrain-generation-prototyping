#include <iostream>
#include <random>
#include <cmath>
#include <optional>
#include <algorithm>

#include <noise/noise.h>
#include <lib/voxel_renderer/voxel_renderer.hpp>

static constexpr auto pi = boost::math::constants::pi<float>();

struct CaveInfo {
    glm::vec3 position;
    glm::vec3 p_direction;
    glm::vec3 s_direction;
    glm::vec3 w_rotation_axis;
    glm::vec3 h_rotation_axis;
    uint32_t length;
    std::vector<uint32_t> branch_points;
    uint32_t layer;

    CaveInfo(
        const glm::vec3 & position_,
        uint32_t length_,
        uint8_t direction_,
        std::vector<uint32_t> branch_points_,
        uint32_t layer_
    ) :
        position(position_),
        length(length_),
        branch_points(branch_points_),
        layer(layer_)
    {
        switch (direction_) {
        case 0: // x
            p_direction = glm::vec3(1.0f, 0.0f, 0.0f);
            s_direction = glm::vec3(0.0f, 1.0f, 0.0f);
            w_rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            h_rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        case 1: // -x
            p_direction = glm::vec3(-1.0f, 0.0f, 0.0f);
            s_direction = glm::vec3(0.0f, -1.0f, 0.0f);
            w_rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            h_rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        case 2: // y
            p_direction = glm::vec3(0.0f, 1.0f, 0.0f);
            s_direction = glm::vec3(1.0f, 0.0f, 0.0f);
            w_rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            h_rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        case 3: // -y
            p_direction = glm::vec3(0.0f, -1.0f, 0.0f);
            s_direction = glm::vec3(-1.0f, 0.0f, 0.0f);
            w_rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            h_rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        }
    }
};

class CaveInfoGenerator {
public:
    static constexpr uint32_t per_chunk = 5u;
    static constexpr uint32_t max_length = 250u;
    static constexpr uint32_t min_length = 20u;
    static constexpr uint32_t max_branches = 4u;
    static constexpr uint32_t min_branches = 0u;
    static constexpr uint32_t max_layer = 2u;
    std::vector<uint8_t> r;

    CaveInfoGenerator(int32_t seed) {
        std::mt19937 mt(seed);
        std::uniform_int_distribution<uint8_t> rand(0u, 255u);
        Helpers::times(256u * 3u, [&](auto) {
            r.push_back(rand(mt));
        });
    }

    std::optional<CaveInfo> make_from_chunk(const glm::vec2 & chunk) const {
        if (int32_t(chunk.x) % per_chunk != 0 || int32_t(chunk.y) % per_chunk != 0) return std::nullopt;
        glm::vec2 center{
            chunk.x * 16u + 8u,
            chunk.y * 16u + 8u
        };
        auto z = chunk_hash(chunk);
        return make_from_point({ center, z }, 0);
    }

    std::optional<CaveInfo> make_from_point(const glm::vec3 & position, uint32_t layer) const {
        if (layer > max_layer) return std::nullopt;
        auto h = position_hash(position);

        uint8_t direction = r[h + 1] % 4;

        auto length_hash = r[h + 2];
        auto max_length_for_current_layer = max_length * std::pow(0.75, layer);
        auto min_length_for_current_layer = min_length;
        auto depth_weight =  0.25f * (127.0f - position.z) / 127.0f;
        uint32_t length = glm::clamp(
            (float)std::round(max_length_for_current_layer * (per(length_hash) + depth_weight)),
            (float)min_length_for_current_layer,
            (float)max_length_for_current_layer
        );

        auto branch_size_hash = r[h + 3];
        auto max_branches_for_current_layer = max_branches * (float(length) / max_length_for_current_layer);
        auto min_branches_for_current_layer = min_branches;
        uint32_t branch_size = glm::clamp(
            (float)std::round(max_branches_for_current_layer * per(branch_size_hash)),
            (float)min_branches_for_current_layer,
            (float)max_branches_for_current_layer
        );

        std::vector<uint32_t> branch_points;

        for (uint32_t i = 1; i <= branch_size; ++i) {
            auto point_hash = r[r[h + 3] + i];
            auto point = std::round(length * per(point_hash));
            branch_points.push_back(point);
        }
        Helpers::unique(branch_points);

        return {{ position, length, direction, branch_points, layer }};
    }

private:
    uint32_t chunk_hash(const glm::vec2 & chunk) const {
        uint8_t cxi = int32_t(chunk.x) % 256u;
        uint8_t cyi = int32_t(chunk.y) % 256u;
        return r[r[cxi] + cyi];
    }

    uint32_t position_hash(const glm::vec3 & position) const {
        auto chunk_x = int32_t(position.x) % 16u;
        auto chunk_y = int32_t(position.y) % 16u;
        uint8_t cxi = chunk_x % 256u;
        uint8_t cyi = chunk_y % 256u;
        uint8_t xi = int32_t(position.x) % 256u;
        uint8_t yi = int32_t(position.y) % 256u;
        uint8_t zi = int32_t(position.z) % 256u;
        return r[r[r[r[r[cxi] + cyi] + xi] + yi] + zi];
    }

    float per(uint8_t hash) const {
        return hash / 255.0f;
    }
};

class CaveGenerator {
    static constexpr auto angle_noise_unit = 300u;
    static constexpr auto radius_noise_unit = 255u;
    static constexpr float max_w_rotation_rad = pi;
    static constexpr float max_h_rotation_rad = pi / 4.0;

    int32_t base_seed;
    uint32_t base_radius;
    CaveInfoGenerator generator;
    noise::module::Perlin angle_noise;
    noise::module::Perlin radius_noise;

public:
    CaveGenerator(int32_t base_seed_) :
        base_seed(base_seed_),
        base_radius(4u),
        generator(base_seed_)
    {
        angle_noise.SetSeed(base_seed + 1);
        angle_noise.SetOctaveCount(5);
        angle_noise.SetFrequency(2.0f / angle_noise_unit);

        radius_noise.SetSeed(base_seed + 2);
        radius_noise.SetOctaveCount(3);
        radius_noise.SetFrequency(8.0f / radius_noise_unit);
    }

    VoxelRenderer::Vertices generate(const glm::vec2 & chunk_from, const glm::vec2 chunk_to) {
        VoxelRenderer::Vertices vertices;

        for (uint32_t x = chunk_from.x; x <= chunk_to.x; ++x) {
            for (uint32_t y = chunk_from.y; y <= chunk_to.y; ++y) {
                auto info = generator.make_from_chunk({ x, y });
                generate_cave(info, vertices);
            }
        }

        return vertices;
    }

private:
    void generate_cave(const std::optional<CaveInfo> & info, VoxelRenderer::Vertices & vertices) {
        using namespace glm;
        using namespace GLHelpers;
        using namespace Helpers;
        if (!info) return;

        times(info->layer, [](auto){ std::cout << "    "; });
        std::cout << boost::format("- %s => %s: [%s]")
            % to_string(info->position)
            % info->length
            % to_string(info->branch_points)
        << std::endl;

        std::vector<float> w_rotations;
        std::vector<float> h_lotations;
        for (uint32_t i = 0; i < info->length; ++i) {
            auto pp = info->position + float(i) * info->p_direction;
            float pv = angle_noise.GetValue(pp.x, pp.y, pp.z);
            pv = clamp(pv, -1.0f, 1.0f);

            auto sp = info->position + float(i) * info->s_direction;
            float sv = angle_noise.GetValue(sp.x, sp.y, sp.z);
            sv = clamp(sv, -1.0f, 1.0f);

            h_lotations.push_back(sv);
            w_rotations.push_back(pv);
        }

        vec3 current_position = info->position;
        float total_length = 0.0f;
        auto branch_points_it = info->branch_points.cbegin();
        auto branch_points_end = info->branch_points.cend();

        for (uint32_t i = 0; i < info->length; ++i) {
            // calculate next position
            mat4 m(1.0f);
            m *= rotate(max_w_rotation_rad * w_rotations[i], info->w_rotation_axis);
            m *= rotate(max_h_rotation_rad * h_lotations[i], info->h_rotation_axis);

            auto next_position = vec3(m * vec4(info->p_direction, 1.0f)) + current_position;
            auto direction = next_position - current_position;
            auto distance = length(direction);
            total_length += distance;

            // make walls
            make_walls(next_position, vertices);

            // fill opening
            {
                uint32_t distance_i = std::ceil(distance);

                for (uint32_t ti = 1; ti < distance_i; ++ti) {
                    auto t = float(ti) / distance_i;
                    auto v = lerp(current_position, next_position, t);
                    make_walls(v, vertices);
                }
            }

            // make branches
            if (branch_points_it != branch_points_end && i == *branch_points_it) {
                ++branch_points_it;
                auto branch_info = generator.make_from_point(next_position, info->layer + 1);
                generate_cave(branch_info, vertices);
            }

            current_position = next_position;
        }

        //std::cout << boost::format("%s's total length: %f") % to_string(info->position) % total_length << std::endl;
    }

    void make_walls(const glm::vec3 & position, VoxelRenderer::Vertices & vertices) {
        float r = base_radius / 2.0f;

        for (int32_t xi = -std::floor(r); xi <= std::floor(r); ++xi) {
            for (int32_t yi = -std::floor(r); yi <= std::floor(r); ++yi) {
                for (int32_t zi = -std::floor(r); zi <= std::floor(r); ++zi) {
                    auto x = position.x + xi;
                    auto y = position.y + yi;
                    auto z = position.z + zi;
                    auto nv = int32_t(base_radius * radius_noise.GetValue(x, y, z));
                    vertices.push_back({ x + nv, y     , z      });
                    vertices.push_back({ x + nv, y + nv, z      });
                    vertices.push_back({ x     , y + nv, z      });
                    vertices.push_back({ x     , y + nv, z + nv });
                    vertices.push_back({ x     , y     , z + nv });
                    vertices.push_back({ x + nv, y     , z + nv });
                }
            }
        }
    }

    glm::vec3 lerp(const glm::vec3 & v1, const glm::vec3 & v2, float t) {
        t = glm::clamp(t, 0.0f, 1.0f);
        return t * (v2 - v1) + v1;
    }
};

int main() {
    try {
        auto window = GLHelpers::init("perlin worms 05");
        VoxelRenderer::Renderer renderer;
        renderer.init(window);

        std::random_device rand_u32;
        auto seed = static_cast<int32_t>(rand_u32());
        //auto seed = 1335689814;
        //auto seed = 660074508;
        //auto seed = -1419309244;
        //auto seed = 473924825;
        //auto seed = 1577647858;
        //auto seed = -272651747;
        //auto seed = -897864757;
        //auto seed = 1671762188;

        std::cout << "Seed: " << seed << std::endl;
        CaveGenerator cave(seed);
        auto vertices = cave.generate({ 0, 0 }, { 20, 20 });
        renderer.render(vertices, [](auto clip) {
            return glm::vec3{
                -2.0f * clip.max.x,
                -2.0f * clip.max.y,
                 4.0f * clip.max.z
            };
        });
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
        return 1;
    }
    return 0;
}
