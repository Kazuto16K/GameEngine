cmake_minimum_required(VERSION 3.28)
project(HelloSFML CXX)

include(FetchContent)

# Network module pulls mbedtls over HTTPS, which fails here (GnuTLS has no CA bundle).
# Not needed for a graphics app, so disable it.
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(sfml GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 3.1.0)
FetchContent_MakeAvailable(sfml)

add_executable(HelloSFML Main.cpp Application.cpp)
target_compile_features(HelloSFML PRIVATE cxx_std_20)
target_link_libraries(HelloSFML PRIVATE SFML::Graphics SFML::Audio)

# Project folder, so assets (background.jpg) load regardless of the working directory
target_compile_definitions(HelloSFML PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
