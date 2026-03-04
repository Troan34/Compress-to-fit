# CMakeList.txt : CMake project for Compress-to-fit, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Compress-to-fit "Compress-to-fit.cpp" "Compress-to-fit.h" "src/util/cli_parser.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Compress-to-fit PROPERTY CXX_STANDARD 23)
endif()

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()
