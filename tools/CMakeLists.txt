add_executable(ryu_table_generator
  ryu_table_generator.cpp
  )
target_include_directories(ryu_table_generator PRIVATE ${TTMATH_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(ryu_table_generator PRIVATE ${TTMATH_DEFINES})
target_link_libraries(ryu_table_generator fmt::fmt)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/include/cache_ryu.h
  COMMAND $<TARGET_FILE:ryu_table_generator> > ${PROJECT_SOURCE_DIR}/include/cache_ryu.h
  DEPENDS ryu_table_generator $<TARGET_FILE:ryu_table_generator>
  )
add_custom_target(generate_ryu_cache ALL DEPENDS ${PROJECT_SOURCE_DIR}/include/cache_ryu.h) 
