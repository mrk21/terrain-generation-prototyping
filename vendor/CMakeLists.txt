include(ExternalProject)

# -----------------------------------------------------------------------------
# Custom target
# -----------------------------------------------------------------------------
add_custom_target(vendor)
add_custom_target(vendor_product)
add_custom_target(vendor_develop)

add_dependencies(vendor vendor_develop)
add_dependencies(vendor_develop vendor_product)

# -----------------------------------------------------------------------------
# Libraries
# -----------------------------------------------------------------------------
## libnoise
ExternalProject_Add(libnoise
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/libnoise
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
ExternalProject_Get_Property(libnoise INSTALL_DIR)
add_dependencies(vendor_product libnoise)

set_property(
  DIRECTORY ${PROJECT_SOURCE_DIR}
  APPEND PROPERTY LINK_DIRECTORIES
    ${INSTALL_DIR}/bin
)
append_project_var(vendor_product_INCLUDE_DIRS
  $<BUILD_INTERFACE:${INSTALL_DIR}/include>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)
append_project_var(vendor_product_LIBRARIES
  noise
)

install(DIRECTORY ${INSTALL_DIR}/include DESTINATION .)
install(DIRECTORY ${INSTALL_DIR}/lib DESTINATION .)

## OpenCV
set_property(
  DIRECTORY ${PROJECT_SOURCE_DIR}
  APPEND PROPERTY LINK_DIRECTORIES
    ${OpenCV_INSTALL_PATH}/lib
)
append_project_var(vendor_product_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
append_project_var(vendor_product_LIBRARIES ${OpenCV_LIBRARIES})

## OpenGL
append_project_var(vendor_product_LIBRARIES ${OPENGL_LIBRARIES})

## GLFW
append_project_var(vendor_product_LIBRARIES glfw)
