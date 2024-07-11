cmake_minimum_required(VERSION 3.19)

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    cmake_path(SET VCPKG_PATH NORMALIZE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH} CACHE STRING "")
endif()

project(SfmlWithCMake VERSION 1.0)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(
	SfmlWithCMake
	${SOURCES}
)
target_link_libraries(
	SfmlWithCMake PRIVATE
	sfml-system sfml-network sfml-graphics sfml-window sfml-main
)

add_custom_command(
	TARGET SfmlWithCMake POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:SfmlWithCMake>/resources
)
