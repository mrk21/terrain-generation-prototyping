#include <iostream>
#include <cmath>
#include <random>
#include <noise/noise.h>
#include <opencv2/opencv.hpp>

void display(const cv::Mat & image) {
    std::string windowName = "windowName";
    cv::namedWindow(windowName);
    cv::imshow(windowName, image);
    cv::waitKey(1000 * 100);
    cv::destroyWindow(windowName);
}

double clamp(double v, double l, double u) {
    if (v < l) return l;
    if (v > u) return u;
    return v;
}

double threshold(double v, double t, double l, double u) {
    return v < t ? l : u;
}

int main() {
    cv::Mat image = cv::Mat::zeros(2000, 2000, CV_8UC3);
    const uint32_t x_length = image.cols;
    const uint32_t y_length = image.rows;

    noise::module::Perlin perlin;
    std::random_device rand_u32;
    perlin.SetSeed(static_cast<int32_t>(rand_u32()));
    perlin.SetOctaveCount(6);
    perlin.SetFrequency(10.0f / 2000.0f);

    std::cout << "seed: " << perlin.GetSeed() << std::endl;

    for (uint32_t x = 0; x < x_length; ++x) {
        for (uint32_t y = 0; y < y_length; ++y) {
            double v = perlin.GetValue(
                1.0 * x,
                1.0 * y,
                0.0
            );
            v = clamp(v, -1.0, 1.0);
            v = (v + 1.0) / 2.0;
            //v = std::abs(v);
            //v = threshold(std::abs(v), 0.05, 0.0, 1.0);
            auto c = static_cast<uint8_t>(v * 255.0);
            image.at<cv::Vec3b>(x,y)[0] = c;
            image.at<cv::Vec3b>(x,y)[1] = c;
            image.at<cv::Vec3b>(x,y)[2] = c;
        }
    }
    display(image);

    return 0;
}
