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
    boost::format dump(const glm::mat4 & m);
}

#endif
