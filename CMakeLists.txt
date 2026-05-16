# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 4.0.3 FATAL_ERROR)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  #set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project ("Compress-to-fit" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_CXX_MODULE_STD ON)
endif()


include(CTest)
include(FetchContent)
enable_testing()
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras") 

FetchContent_Declare(
    mio
    GIT_REPOSITORY https://github.com/vimpunk/mio
    GIT_TAG master
)
FetchContent_MakeAvailable(mio)

# Include sub-projects.
add_subdirectory ("Compress-to-fit")

if(MSVC)
    target_link_options(Compress-to-fit PRIVATE "/STACK:10485760")
    target_compile_options(Compress-to-fit PRIVATE "/utf-8")
elseif(APPLE)
    target_link_options(Compress-to-fit PRIVATE "LINKER:-stack_size,0x1000000")
elseif(WIN32 AND NOT MSVC) #MinGW
    target_link_options(Compress-to-fit PRIVATE "-Wl,--stack,10485760")
else() #Linux
    target_link_options(Compress-to-fit PRIVATE "LINKER:-z,stack-size=10485760")
endif()

find_package(Doxygen OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_FILE_PATTERNS "*.cpp;*.ixx;*.cppm;*.h;*.hpp")
    set(DOXYGEN_EXTENSION_MAPPING "ixx=C++ cppm=C++")

    doxygen_add_docs(
        docs
        ${PROJECT_SOURCE_DIR}/src
        COMMENT "Generating documentation with Doxygen"
    )
endif()

