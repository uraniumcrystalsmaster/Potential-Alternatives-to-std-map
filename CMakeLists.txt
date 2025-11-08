cmake_minimum_required(VERSION 3.22)
project(Fast_Tries_and_Radix_Sort)

set(CMAKE_CXX_STANDARD 14)

#compile flags to match Gradescope test environment
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Werror")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

#Don't build SFML examples
set(SFML_BUILD_EXAMPLES OFF CACHE INTERNAL "")

## --- pull in desired version of catch ---
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(extern/sfml)

# --- Define Executables ---
add_executable(Main
        src/main.cpp
)

add_executable(Tests
        test/test.cpp
)

# --- Add Include Paths to Targets ---
# We ONLY need to add our project's headers.
# SFML's headers are added automatically by target_link_libraries.
target_include_directories(Main PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        submodules/FunnelHashMap/src
)

target_include_directories(Tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        submodules/FunnelHashMap/src
)

target_link_libraries(Main PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain)

# --- OS-Specific DLL Copy (for Windows ONLY) ---
# This block will be correctly skipped on your Mac.
# On Windows, it copies the DLLs from your C: drive.
if(WIN32)
    add_custom_command(TARGET Main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/SFML-2.5.1/bin/sfml-graphics-d-2.dll"
            $<TARGET_FILE_DIR:Main>
    )
    add_custom_command(TARGET Main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/SFML-2.5.1/bin/sfml-window-d-2.dll"
            $<TARGET_FILE_DIR:Main>
    )
    add_custom_command(TARGET Main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/SFML-2.5.1/bin/sfml-system-d-2.dll"
            $<TARGET_FILE_DIR:Main>
    )
endif()

# comment everything below out if you are using CLion
#include(CTest)
#include(Catch)
#catch_discover_tests(Tests)
