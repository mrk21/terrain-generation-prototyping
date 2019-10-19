#include <iostream>
#include <random>
#include <cmath>
#include <noise/noise.h>
#include <lib/voxel_renderer/voxel_renderer.hpp>

class CaveGenerator {
public:
    uint32_t x_length;
    uint32_t y_length;
    uint32_t z_length;

    CaveGenerator(uint32_t x_length_, uint32_t y_length_, uint32_t z_length_):
        x_length(x_length_),
        y_length(y_length_),
        z_length(z_length_)
    {}

    float value_to_angle(float value) {
        static const auto pi = boost::math::constants::pi<float>();
        return value * pi / 16.0f;
    }

    VoxelRender::Vertices generate(uint32_t cave_size) {
        VoxelRender::Vertices vertices;
        std::random_device rand_u32;
        std::mt19937 mt{ rand_u32() };
        std::uniform_int_distribution<uint32_t> rx(0u, x_length);
        std::uniform_int_distribution<uint32_t> ry(0u, y_length);
        std::uniform_int_distribution<uint32_t> rz(0u, z_length);

        noise::module::Perlin perlin;
        perlin.SetSeed(static_cast<int32_t>(rand_u32()));
        perlin.SetOctaveCount(3);
        perlin.SetFrequency(4.0f);

        auto xy_length = std::min(x_length, y_length);

        Helpers::times(cave_size / 2, [&](auto){
            uint32_t ox = rx(mt);
            uint32_t oy = ry(mt);
            uint32_t oz = rz(mt);

            std::vector<float> x_points;
            for (uint32_t x = ox; x < xy_length + ox; ++x) {
                float v = perlin.GetValue(
                    1.0 *  x / xy_length,
                    1.0 * oy / xy_length,
                    1.0 * oz / z_length
                );
                v = Helpers::clamp(v, -1.0f, 1.0f);
                x_points.push_back(v);
            }

            std::vector<float> y_points;
            for (uint32_t y = oy; y < xy_length + oy; ++y) {
                float v = perlin.GetValue(
                    1.0 * ox / xy_length,
                    1.0 *  y / xy_length,
                    1.0 * oz / z_length
                );
                v = Helpers::clamp(v, -1.0f, 1.0f);
                y_points.push_back(v);
            }

            for (uint32_t x = 0; x < xy_length; ++x) {
                auto mat =
                    glm::rotate(value_to_angle(y_points[x]), glm::vec3(1.0f, 0.0f, 0.0f)) *
                    glm::rotate(value_to_angle(x_points[x]), glm::vec3(0.0f, 0.0f, 1.0f))
                ;
                auto vec =  mat * glm::vec4(ox + x, oy, oz, 1.0f);

                for (int32_t x = -4; x <= 4; ++x) {
                    for (int32_t y = -4; y <= 4; ++y) {
                        for (int32_t z = -4; z <= 4; ++z) {
                            vertices.push_back({
                                std::round(vec.x) + x,
                                std::round(vec.y) + y,
                                std::round(vec.z) + z
                            });
                        }
                    }
                }
            }
        });

        Helpers::times(cave_size / 2, [&](auto){
            uint32_t ox = rx(mt);
            uint32_t oy = ry(mt);
            uint32_t oz = rz(mt);

            std::vector<float> x_points;
            for (uint32_t x = ox; x < xy_length + ox; ++x) {
                float v = perlin.GetValue(
                    1.0 *  x / xy_length,
                    1.0 * oy / xy_length,
                    1.0 * oz / z_length
                );
                v = Helpers::clamp(v, -1.0f, 1.0f);
                x_points.push_back(v);
            }

            std::vector<float> y_points;
            for (uint32_t y = oy; y < xy_length + oy; ++y) {
                float v = perlin.GetValue(
                    1.0 * ox / xy_length,
                    1.0 *  y / xy_length,
                    1.0 * oz / z_length
                );
                v = Helpers::clamp(v, -1.0f, 1.0f);
                y_points.push_back(v);
            }

            for (uint32_t y = 0; y < xy_length; ++y) {
                auto mat =
                    glm::rotate(value_to_angle(y_points[y]), glm::vec3(0.0f, 1.0f, 0.0f)) *
                    glm::rotate(value_to_angle(x_points[y]), glm::vec3(0.0f, 0.0f, 1.0f))
                ;
                auto vec = mat * glm::vec4(ox, oy + y, oz, 1.0f);

                for (int32_t x = -4; x <= 4; ++x) {
                    for (int32_t y = -4; y <= 4; ++y) {
                        for (int32_t z = -4; z <= 4; ++z) {
                            vertices.push_back({
                                std::round(vec.x) + x,
                                std::round(vec.y) + y,
                                std::round(vec.z) + z
                            });
                        }
                    }
                }
            }
        });
        std::sort(vertices.begin(), vertices.end());
        auto result = std::unique(vertices.begin(), vertices.end());
        vertices.erase(result, vertices.end());

        return vertices;
    }
};

int main() {
    try {
        auto window = GLHelpers::init("perlin worms 3d");
        VoxelRender::Renderer renderer;
        renderer.init(window);

        CaveGenerator cave(400, 400, 400);
        auto vertices = cave.generate(13u);
        std::cout << "vertex count: " << vertices.size() << std::endl;

        renderer.render(vertices, { cave.x_length, cave.y_length, cave.z_length });
        return 0;
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
    }
}