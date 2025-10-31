cmake_minimum_required(VERSION 3.22)
project(Fast_Tries_and_Radix_Sort)

set(CMAKE_CXX_STANDARD 14)

#compile flags to match Gradescope test environment
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

## --- pull in desired version of catch ---
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

set(CMAKE_PREFIX_PATH "C:/SFML-2.5.1" ${CMAKE_PREFIX_PATH})

# This command finds SFML. We will manually link to its files.
find_package(SFML 2.5 REQUIRED graphics)

# --- Define Executables ---
# Built from SOURCE (.cpp) files ONLY
add_executable(Main
        src/main.cpp
)

add_executable(Tests
        test/test.cpp
)

# --- Add Include Paths to Targets ---
# This tells the compiler where to find #included headers for EACH target
target_include_directories(Main PRIVATE
        ${PROJECT_SOURCE_DIR}/src       # Path to X-fast_Trie.h
        submodules/FunnelHashMap/src  # Path to Funnel_Hash_Map.h
        "C:/SFML-2.5.1/include"         # Use the explicit path, not ${SFML_DIR}
)

target_include_directories(Tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src       # Path to X-fast_Trie.h
        submodules/FunnelHashMap/src  # Path to Funnel_Hash_Map.h
)
# Use explicit paths to the libraries, not ${SFML_DIR}
target_link_libraries(Main PRIVATE
        "C:/SFML-2.5.1/lib/libsfml-graphics-d.a"
        "C:/SFML-2.5.1/lib/libsfml-window-d.a"
        "C:/SFML-2.5.1/lib/libsfml-system-d.a"
)
# --- Link Libraries ---
target_link_libraries(Tests PRIVATE Catch2::Catch2WithMain)

# Use explicit paths for the DLLs, not ${SFML_DIR}
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

# comment everything below out if you are using CLion
#include(CTest)
#include(Catch)
#catch_discover_tests(Tests)

