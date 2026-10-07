cmake_minimum_required(VERSION 3.10)
project(TheLastClue CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Every .cpp under src/ is compiled into one library automatically —
# just add your file in the right folder, no need to edit this file.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_library(lastclue STATIC ${ENGINE_SOURCES})
target_include_directories(lastclue PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Every demos/<name>.cpp becomes an executable called <name>.
file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/demos/*.cpp)
foreach(demo ${DEMO_SOURCES})
    get_filename_component(name ${demo} NAME_WE)
    add_executable(${name} ${demo})
    target_link_libraries(${name} PRIVATE lastclue)
endforeach()
