cmake_minimum_required(VERSION 3.14)
project(TheLastClue CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Engine: every .cpp under src/ (except the window layer in src/platform/) is
# compiled into one library automatically — add your file in the right folder,
# no need to edit this file. This part needs NO graphics library.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/src/platform/.*")
add_library(lastclue STATIC ${ENGINE_SOURCES})
target_include_directories(lastclue PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Every demos/<name>.cpp becomes an executable called <name> (writes images to output/).
file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/demos/*.cpp)
foreach(demo ${DEMO_SOURCES})
    get_filename_component(name ${demo} NAME_WE)
    add_executable(${name} ${demo})
    target_link_libraries(${name} PRIVATE lastclue)
endforeach()

# ---------------------------------------------------------------------------
# CP3: the OpenGL game window (GLFW + OpenGL).
# GLFW is used if it is installed; otherwise CMake downloads it from GitHub
# the first time you configure (needs internet once).
# Turn it off with:  cmake -B build -S . -DLASTCLUE_GAME=OFF
# ---------------------------------------------------------------------------
option(LASTCLUE_GAME "Build the OpenGL game window (apps/)" ON)
if(LASTCLUE_GAME)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 QUIET)
    if(NOT glfw3_FOUND)
        message(STATUS "GLFW not installed - downloading GLFW 3.4")
        include(FetchContent)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)   # Linux: X11 only (fewer dependencies)
        FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(glfw)
    endif()

    file(GLOB PLATFORM_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/platform/*.cpp)
    add_library(lastclue_platform STATIC ${PLATFORM_SOURCES})
    target_link_libraries(lastclue_platform PUBLIC lastclue glfw OpenGL::GL)
    if(APPLE)
        target_compile_definitions(lastclue_platform PUBLIC GL_SILENCE_DEPRECATION)
    endif()

    # Every apps/<name>.cpp becomes a windowed program called <name>.
    file(GLOB APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/apps/*.cpp)
    foreach(app ${APP_SOURCES})
        get_filename_component(name ${app} NAME_WE)
        add_executable(${name} ${app})
        target_link_libraries(${name} PRIVATE lastclue_platform)
    endforeach()
endif()
