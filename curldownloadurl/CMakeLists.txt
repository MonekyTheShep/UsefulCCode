cmake_minimum_required(VERSION 4.1)
project(curldownloadurl C)

set(CMAKE_C_STANDARD 11)

# Find libcurl installed on the system
find_package(CURL REQUIRED)
find_package(cjson REQUIRED)

add_executable(curldownloadurl main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE cjson)


target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -fsanitize=address
        -g
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# Link libcurl to your executable
target_link_libraries(${PROJECT_NAME}
        PRIVATE
            cjson
            CURL::libcurl
)
