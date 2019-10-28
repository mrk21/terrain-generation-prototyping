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
    GLFWwindow * init(const std::string & title, GLuint width = 1280, GLuint height = 960);
    std::string to_string(const glm::mat4 & m);
    std::string to_string(const glm::vec3 & v);
}

#endif
