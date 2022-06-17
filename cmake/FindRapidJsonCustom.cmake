find_package(RapidJSON QUIET)

if(NOT (RAPIDJSON_FOUND))
  configure_file(${CMAKE_SOURCE_DIR}/cmake/rapidjson.txt.in
    ${CMAKE_BINARY_DIR}/rapidjson-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/rapidjson-download
  )
  if(result)
    message(FATAL_ERROR "CMake step for rapidjson failed: ${result}")
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/rapidjson-download)
  if(result)
    message(FATAL_ERROR "Build step for rapidjson failed: ${result}")
  endif()

  set(RapidJSON_DIR ${CMAKE_BINARY_DIR}/rapidjson-src/include)
  include_directories(${RapidJSON_DIR})

  # Add rapidjson directly to our build.
#  add_subdirectory(
#    ${CMAKE_BINARY_DIR}/rapidjson-src
#    ${CMAKE_BINARY_DIR}/rapidjson-build
#    EXCLUDE_FROM_ALL)
endif()