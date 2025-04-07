cmake_minimum_required(VERSION 3.6)
project(bowProject)

set(EXEC_NAME bow)

# Set GMSH Version : example gmsh-4.12.2 -> GMSH_VERSION=gmsh-4.12, GMSH_MINOR=2
set(GMSH_VERSION gmsh-4.13)
set(GMSH_MINOR 1)


# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# GLFW
add_subdirectory(glfw)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)

# GMSH
if(MSVC OR MSYS OR MINGW)
    set(GMSH_LIBRARY ${CMAKE_SOURCE_DIR}/gmsh/${GMSH_VERSION}.${GMSH_MINOR}-Windows64-sdk/lib/gmsh.lib)
    set(GMSH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/gmsh/${GMSH_VERSION}.${GMSH_MINOR}-Windows64-sdk/include)
    #Copy .dll into Debug
    set(GMSH_DLL ${CMAKE_SOURCE_DIR}/gmsh/${GMSH_VERSION}.${GMSH_MINOR}-Windows64-sdk/lib/${GMSH_VERSION}.dll)
    if(MSVC)
        file(COPY ${GMSH_DLL} DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    else()
        file(COPY ${GMSH_DLL} DESTINATION ${CMAKE_BINARY_DIR})
    endif()
endif(MSVC OR MSYS OR MINGW)

# link the aforementioned libraries
add_executable(${EXEC_NAME} src/glfem.c src/main.c src/utils.c)
target_link_libraries(${EXEC_NAME} glfw ${OPENGL_gl_LIBRARY} ${GMSH_LIBRARY})

if(APPLE)
    target_compile_options(${EXEC_NAME} PRIVATE "-Wdeprecated-declarations")
    target_compile_definitions(${EXEC_NAME} PRIVATE -D GL_SILENCE_DEPRECATION)
endif()

# set exec as the startup project in visual studio
if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC_NAME})
    target_compile_options(${EXEC_NAME} PRIVATE "/W2")
endif()

target_compile_definitions(${EXEC_NAME} PRIVATE graphic)

# add a custom target so we can do make run directly
add_custom_target(run COMMAND ${EXEC_NAME} DEPENDS ${EXEC_NAME} COMMENT "./${EXEC_NAME}")

