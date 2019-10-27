#include <iostream>
#include <random>
#include <cmath>
#include <noise/noise.h>
#include <lib/voxel_renderer/voxel_renderer.hpp>
#include <optional>

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

    friend std::ostream & operator <<(std::ostream & os, const CaveInfo & info) {
        os << "position: " << GLHelpers::dump(info.position) << "\n";
        os << "p_direction: " << GLHelpers::dump(info.p_direction) << "\n";
        os << "s_direction: " << GLHelpers::dump(info.s_direction) << "\n";
        os << "w_rotation_axis: " << GLHelpers::dump(info.w_rotation_axis) << "\n";
        os << "h_rotation_axis: " << GLHelpers::dump(info.h_rotation_axis) << "\n";
        os << "length: " << info.length << "\n";
        os << "branch_points: (";
        for (auto p: info.branch_points) os << p << ", ";
        os << ")\n";
        return os;
    }
};

class CaveInfoGenerator {
    static constexpr uint32_t per_chunk = 3u;
    static constexpr uint32_t max_length = 400u;
    static constexpr uint32_t min_length = 50u;
    static constexpr uint32_t max_branches = 5u;
    static constexpr uint32_t min_branches = 0u;
    static constexpr uint32_t max_layer = 1u;
    std::vector<uint8_t> r;

public:
    CaveInfoGenerator(int32_t seed) {
        std::mt19937 mt(seed);
        std::uniform_int_distribution<uint8_t> rand(0u, 255u);
        Helpers::times(256u * 3u, [&](auto) {
            r.push_back(rand(mt));
        });
    }

    std::optional<CaveInfo> make_from_chunk(const glm::vec2 & p) const {
        if (int32_t(p.x) % per_chunk != 0 || int32_t(p.y) % per_chunk != 0) return std::nullopt;

        glm::vec2 c{
            p.x * 16u + 8u,
            p.y * 16u + 8u
        };
        uint8_t cxi = static_cast<uint32_t>(c.x) % 256u;
        uint8_t cyi = static_cast<uint32_t>(c.y) % 256u;
        auto z = r[r[cxi] + cyi];
        return make_from_point({ c, z }, 0);
    }

    std::optional<CaveInfo> make_from_point(const glm::vec3 & position, uint32_t layer) const {
        if (layer > max_layer) return std::nullopt;

        uint8_t xi = static_cast<uint32_t>(position.x) % 256u;
        uint8_t yi = static_cast<uint32_t>(position.y) % 256u;
        uint8_t zi = static_cast<uint32_t>(position.z) % 256u;
        uint8_t direction = r[r[r[xi] + yi] + zi] % 4;

        auto length_hash = r[r[r[r[xi] + yi] + zi] + 1];
        auto max_length_for_current_layer = max_length * std::pow(0.75, layer);
        auto min_length_for_current_layer = min_length;
        uint32_t length = glm::clamp(
            (float)std::round(max_length_for_current_layer * per(length_hash)),
            (float)min_length_for_current_layer,
            (float)max_length_for_current_layer
        );

        auto branch_size_hash = r[r[r[r[xi] + yi] + zi] + length_hash];
        auto max_branches_for_current_layer = max_branches * (float(length) / max_length_for_current_layer);
        auto min_branches_for_current_layer = min_branches;
        uint32_t branch_size = glm::clamp(
            (float)std::round(max_branches_for_current_layer * per(branch_size_hash)),
            (float)min_branches_for_current_layer,
            (float)max_branches_for_current_layer
        );

        std::vector<uint32_t> branch_points;

        for (uint32_t i = 1; i <= branch_size; ++i) {
            auto point_hash = r[r[r[r[r[xi] + yi] + zi] + branch_size_hash] + i];
            auto point = std::round(length * per(point_hash));
            branch_points.push_back(point);
        }
        std::sort(branch_points.begin(), branch_points.end());
        branch_points.erase(std::unique(branch_points.begin(), branch_points.end()), branch_points.end());

        return {{ position, length, direction, branch_points, layer }};
    }

private:
    float per(uint8_t hash) const {
        return hash / 255.0f;
    }
};

class CaveGenerator {
    int32_t base_seed;
    uint32_t base_radius;
    noise::module::Perlin angle_noise;
    noise::module::Perlin radius_noise;

public:
    CaveGenerator(int32_t base_seed_) : base_seed(base_seed_) {
        base_radius = 4u;

        angle_noise.SetSeed(base_seed + 1);
        angle_noise.SetOctaveCount(10);
        angle_noise.SetFrequency(4.0f);

        radius_noise.SetSeed(base_seed + 2);
        radius_noise.SetOctaveCount(3);
        radius_noise.SetFrequency(8.0f);
    }

    VoxelRenderer::Vertices generate(const glm::vec2 & from, const glm::vec2 to) {
        VoxelRenderer::Vertices vertices;
        CaveInfoGenerator generator(base_seed);

        for (uint32_t x = from.x; x <= to.x; ++x) {
            for (uint32_t y = from.y; y <= to.y; ++y) {
                auto info = generator.make_from_chunk({ x, y });
                generate_sub(generator, info, vertices);
            }
        }

        return vertices;
    }

private:
    float value_to_angle(float value) {
        static const auto pi = boost::math::constants::pi<float>();
        return value * pi / 2.0f;
    }

    void generate_sub(const CaveInfoGenerator & generator, const std::optional<CaveInfo> & info, VoxelRenderer::Vertices & vertices) {
        if (!info) return;
        //std::cout << *info << std::endl;

        std::vector<float> w_rotations;
        for (uint32_t i = 0; i < info->length; ++i) {
            auto p = info->position + 1.0f * i * info->p_direction;
            float v = angle_noise.GetValue(
                1.0 * p.x / info->length,
                1.0 * p.y / info->length,
                1.0 * p.z / 256.0
            );
            v = Helpers::clamp(v, -1.0f, 1.0f);
            w_rotations.push_back(v);
        }

        std::vector<float> h_lotations;
        for (uint32_t i = 0; i < info->length; ++i) {
            auto p = info->position + 1.0f * i * info->s_direction;
            float v = angle_noise.GetValue(
                1.0 * p.x / info->length,
                1.0 * p.y / info->length,
                1.0 * p.z / 256.0
            );
            v = Helpers::clamp(v, -1.0f, 1.0f);
            v *= 0.5f;
            h_lotations.push_back(v);
        }

        glm::vec3 current_position = info->position;
        float total_length = 0.0f;

        auto branch_points_it = info->branch_points.cbegin();
        auto branch_points_end = info->branch_points.cend();

        for (uint32_t i = 0; i < info->length; ++i) {
            // calculate next position
            glm::mat4 m(1.0f);
            m *= glm::rotate(value_to_angle(h_lotations[i]), info->h_rotation_axis);
            m *= glm::rotate(value_to_angle(w_rotations[i]), info->w_rotation_axis);

            auto next_position = glm::vec3(m * glm::vec4(info->p_direction, 1.0f)) + current_position;
            auto direction = next_position - current_position;
            auto distance = glm::length(direction);
            total_length += distance;

            // make walls
            make_walls(next_position, vertices);

            // fill opening
            {
                uint32_t distance_i = std::ceil(distance);

                for (uint32_t ti = 1; ti < distance_i; ++ti) {
                    auto t = (1.0f * ti) / distance_i;
                    auto v = lerp(current_position, next_position, t);
                    make_walls(v, vertices);
                }
            }

            // make branches
            if (branch_points_it != branch_points_end && i == *branch_points_it) {
                ++branch_points_it;
                auto sub_info = generator.make_from_point(next_position, info->layer + 1);
                generate_sub(generator, sub_info, vertices);
            }

            current_position = next_position;
        }

        std::cout << boost::format("%s's total length: %f") % GLHelpers::dump(info->position) % total_length << std::endl;
    }

    void make_walls(const glm::vec3 & position, VoxelRenderer::Vertices & vertices) {
        float r = base_radius / 2.0f;

        auto noise_v = [this](int32_t x, int32_t y, int32_t z) {
            float v = radius_noise.GetValue(
                x / 256.0,
                y / 256.0,
                z / 256.0
            );
            return base_radius * v;
        };

        for (int32_t xi = -std::floor(r); xi <= std::floor(r); ++xi) {
            for (int32_t yi = -std::floor(r); yi <= std::floor(r); ++yi) {
                for (int32_t zi = -std::floor(r); zi <= std::floor(r); ++zi) {
                    auto x = position.x + xi;
                    auto y = position.y + yi;
                    auto z = position.z + zi;
                    auto nv = noise_v(x, y, z);
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

        std::cout << "Seed: " << seed << std::endl;
        CaveGenerator cave(seed);
        auto vertices = cave.generate({ 0, 0 }, { 6, 6 });
        renderer.render(vertices);
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
        return 1;
    }
    return 0;
}
