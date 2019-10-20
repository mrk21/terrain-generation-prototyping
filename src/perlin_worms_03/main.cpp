#include <iostream>
#include <random>
#include <noise/noise.h>
#include <lib/voxel_renderer/voxel_renderer.hpp>

double clamp(double v, double l, double u) {
    if (v < l) return l;
    if (v > u) return u;
    return v;
}

double threshold(double v, double t, double l, double u) {
    return v < t ? l : u;
}

class NoiseGenerator {
public:
    uint32_t x_length;
    uint32_t y_length;
    uint32_t z_length;

    NoiseGenerator(uint32_t x_length_, uint32_t y_length_, uint32_t z_length_):
        x_length(x_length_),
        y_length(y_length_),
        z_length(z_length_)
    {}

    VoxelRenderer::Vertices generate() {
        VoxelRenderer::Vertices vertices;
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
                        vertices.push_back({
                            1.0f * x,
                            1.0f * y,
                            1.0f * z
                        });
                    }
                }
            }
        }

        return vertices;
    }
};

int main() {
    try {
        auto window = GLHelpers::init("perlin noise 3d");
        VoxelRenderer::Renderer renderer;
        renderer.init(window);

        NoiseGenerator noise(100, 100, 100);
        auto vertices = noise.generate();
        std::cout << "vertex count: " << vertices.size() << std::endl;

        renderer.render(vertices, { noise.x_length, noise.y_length, noise.z_length });
        return 0;
    }
    catch (std::string str) {
        std::cerr << str << std::endl;
    }
}
