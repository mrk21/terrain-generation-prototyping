add_executable(cave_wall_01 main.cpp)
target_include_directories(cave_wall_01 PUBLIC ${vendor_product_INCLUDE_DIRS})
target_link_libraries(cave_wall_01 PUBLIC ${vendor_product_LIBRARIES} lib)
