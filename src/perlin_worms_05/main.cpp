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

    CaveInfo(const glm::vec3 & position_, uint32_t length_, uint8_t direction_) : position(position_), length(length_) {
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
    std::vector<uint8_t> r;

public:
    CaveInfoGenerator(int32_t seed) {
        std::mt19937 mt(seed);
        std::uniform_int_distribution<uint8_t> rand(0u, 255u);
        Helpers::times(256u * 3u, [&](auto){
            r.push_back(rand(mt));
        });
    }

    std::optional<CaveInfo> make_from_chunk(const glm::vec2 & p, uint32_t l) const {
        glm::vec2 c{
            p.x * 16u + 8u,
            p.y * 16u + 8u
        };
        uint8_t cxi = static_cast<uint32_t>(c.x) % 256u;
        uint8_t cyi = static_cast<uint32_t>(c.y) % 256u;
        auto z = r[r[cxi] + cyi];
        return make_from_point({ c, z }, l);
    }

    std::optional<CaveInfo> make_from_point(const glm::vec3 & p, uint32_t l) const {
        uint8_t xi = static_cast<uint32_t>(p.x) % 256u;
        uint8_t yi = static_cast<uint32_t>(p.y) % 256u;
        uint8_t zi = static_cast<uint32_t>(p.z) % 256u;
        uint8_t d = r[r[r[xi] + yi] + zi] % 4;
        return {{ p, l, d }};
    }
};

class CaveGenerator {
    int32_t base_seed;
    uint32_t base_length;
    uint32_t base_radius;
    noise::module::Perlin angle_noise;
    noise::module::Perlin radius_noise;

public:
    CaveGenerator(int32_t base_seed_, uint32_t base_length_) : base_seed(base_seed_), base_length(base_length_) {
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
                auto info = generator.make_from_chunk({ x, y }, base_length);
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
        if (info->length < (base_length / 2)) return;

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

        for (uint32_t i = 0; i < info->length; ++i) {
            // calculate next position
            glm::mat4 m(1.0f);
            m *= glm::translate(current_position);
            m *= glm::rotate(value_to_angle(h_lotations[i]), info->h_rotation_axis);
            m *= glm::rotate(value_to_angle(w_rotations[i]), info->w_rotation_axis);
            m *= glm::translate(info->p_direction);
            m *= glm::translate(-current_position);

            auto next_position = glm::vec3(m * glm::vec4(current_position, 1.0f));
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
            if (i % (info->length / 4) == 0) {
                auto sub_info = generator.make_from_point(next_position, info->length / 2);
                generate_sub(generator, sub_info, vertices);
            }

            current_position = next_position;
        }

        std::cout << boost::format("%s's total length: %f") % GLHelpers::dump(info->position) % total_length << std::endl;
    }

    void make_walls(const glm::vec3 & position, VoxelRenderer::Vertices & vertices) {
        int32_t r = base_radius / 2;

        auto noise_v = [this](int32_t x, int32_t y, int32_t z) {
            float v = radius_noise.GetValue(
                x / 256.0,
                y / 256.0,
                z / 256.0
            );
            return base_radius * v;
        };

        for (int32_t xi = -r; xi <= r; ++xi) {
            for (int32_t yi = -r; yi <= r; ++yi) {
                for (int32_t zi = -r; zi <= r; ++zi) {
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

        std::cout << "Seed: " << seed << std::endl;
        CaveGenerator cave(seed, 400u);
        auto vertices = cave.generate({ 0, 0 }, { 1, 1 });
        renderer.render(vertices);
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
        return 1;
    }
    return 0;
}
