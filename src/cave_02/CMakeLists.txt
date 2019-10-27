add_executable(cave_02 main.cpp)
target_include_directories(cave_02 PUBLIC ${vendor_product_INCLUDE_DIRS})
target_link_libraries(cave_02 PUBLIC ${vendor_product_LIBRARIES} lib)
