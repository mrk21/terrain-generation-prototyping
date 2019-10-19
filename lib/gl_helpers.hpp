#ifndef GL_HELPERS_HPP
#define GL_HELPERS_HPP

#include <iostream>
#include <boost/format.hpp>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <lib/helpers.hpp>

namespace GLHelpers {
    GLFWwindow * init(const std::string & title, GLuint width = 640, GLuint height = 480) {
        if (!glfwInit()) {
            std::cerr << "glfwInit failed." << std::endl;
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow * window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            std::cerr << "glfwCreateWindow failed." << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        std::cout << "## OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        return window;
    }

    auto dump(const glm::mat4 & m) {
        return boost::format(Helpers::trim_base_indent(R"(
            % 10.5f   % 10.5f   % 10.5f   % 10.5f
            % 10.5f   % 10.5f   % 10.5f   % 10.5f
            % 10.5f   % 10.5f   % 10.5f   % 10.5f
            % 10.5f   % 10.5f   % 10.5f   % 10.5f
        )"))
            % m[0][0] % m[0][1] % m[0][2] % m[0][3]
            % m[1][0] % m[1][1] % m[1][2] % m[1][3]
            % m[2][0] % m[2][1] % m[2][2] % m[2][3]
            % m[3][0] % m[3][1] % m[3][2] % m[3][3]
        ;
    }
}

#endif
