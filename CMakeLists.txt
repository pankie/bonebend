cmake_minimum_required(VERSION 3.20)
project(bonebend C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.14   # bump this to a newer release-x.y.z tag as needed
)
FetchContent_MakeAvailable(SDL3)

add_executable(bonebend main.c
        vec.c
        vec.h
        mat4.c
        mat4.h
        mesh.c
        mesh.h
        triangle.c
        triangle.h
        skeleton.c
        skeleton.h)

target_link_libraries(bonebend PRIVATE SDL3::SDL3)


# Special treatment for Windows:
# copies the DLL file to the executable after each build
if(WIN32)
    add_custom_command(
            TARGET bonebend POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:bonebend>
            VERBATIM
    )
endif()