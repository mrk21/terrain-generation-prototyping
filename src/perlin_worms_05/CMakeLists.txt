add_executable(perlin_worms_05 main.cpp)
target_include_directories(perlin_worms_05 PUBLIC ${vendor_product_INCLUDE_DIRS})
target_link_libraries(perlin_worms_05 PUBLIC ${vendor_product_LIBRARIES} lib)
