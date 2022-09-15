find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()
find_package(httplib QUIET)

if(NOT (HTTPLIB_FOUND))
  configure_file(${CMAKE_SOURCE_DIR}/cmake/httplib.txt.in
    ${CMAKE_BINARY_DIR}/httplib-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/httplib-download)
  if(result)
    message(FATAL_ERROR "CMake step for httplib failed: ${result}")
  endif()

  # Different command on Linux apparently
  execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/httplib-download)
  if(result)
    message(FATAL_ERROR "Build step for httplib failed: ${result}")
  endif()

  set(httplib_DIR ${CMAKE_BINARY_DIR}/httplib-src)
  include_directories(${httplib_DIR})

  # Add httplib directly to our build.
#  add_subdirectory(${CMAKE_BINARY_DIR}/httplib-src
#    ${CMAKE_BINARY_DIR}/httplib-build
#    EXCLUDE_FROM_ALL)
endif()

#target_precompile_headers(httplib::httplib INTERFACE "${HTTPLIB_HEADER_PATH}")