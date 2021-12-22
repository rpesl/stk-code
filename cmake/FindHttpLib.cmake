find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()
find_package(httplib REQUIRED)
target_precompile_headers(httplib::httplib INTERFACE "${HTTPLIB_HEADER_PATH}")
