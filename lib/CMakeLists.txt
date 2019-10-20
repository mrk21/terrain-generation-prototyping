file(GLOB_RECURSE SRCS "**.cpp")

add_library(lib STATIC ${SRCS})
target_include_directories(lib PUBLIC ${vendor_product_INCLUDE_DIRS})
target_link_libraries(lib PUBLIC ${vendor_product_LIBRARIES})
