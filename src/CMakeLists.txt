file(GLOB SRCS "**.cpp" "*/**.cpp")

foreach(f ${SRCS})
  get_filename_component(bf ${f} NAME_WE)
  add_executable(${bf} ${f})
  target_include_directories(${bf} PUBLIC ${vendor_product_INCLUDE_DIRS})
  target_link_libraries(${bf} PUBLIC ${vendor_product_LIBRARIES})
endforeach()
