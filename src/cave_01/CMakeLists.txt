add_executable(cave_01 main.cpp)
target_include_directories(cave_01 PUBLIC ${vendor_product_INCLUDE_DIRS})
target_link_libraries(cave_01 PUBLIC ${vendor_product_LIBRARIES} lib)
