# CMakeList.txt : CMake project for Compress-to-fit, include source and define
# project specific logic here.
#

add_subdirectory(modules/std)

add_library(project_logic)

target_link_libraries(project_logic PRIVATE std_module)

target_sources(
    project_logic
    PRIVATE
    FILE_SET CXX_MODULES FILES
    "src/util/utils.cppm"
    "src/util/cli_parser.cppm"
    "src/util/file_management.cppm"
    "src/models/LZ77/lz77.cppm"
    "src/models/LZ77/lz77_impl.cppm"
    "src/util/concurrent_file_buffer.cppm")

target_sources(
    project_logic
    PRIVATE
    "src/util/cli_parser.cpp"
)

FetchContent_GetProperties(mio SOURCE_DIR mio_SOURCE_DIR)
target_include_directories(project_logic PUBLIC "${mio_SOURCE_DIR}/include")
target_compile_definitions(project_logic PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)

add_executable(Compress-to-fit)
set_target_properties(Compress-to-fit PROPERTIES OUTPUT_NAME "ctf")
target_link_libraries(Compress-to-fit PRIVATE project_logic)
target_link_libraries(project_logic PRIVATE std_module)
target_sources(Compress-to-fit PRIVATE "Compress-to-fit.cpp")
target_compile_features(Compress-to-fit PRIVATE cxx_std_23)

add_executable(tests)
target_link_libraries(tests PRIVATE 
    project_logic
    Catch2::Catch2WithMain
    mio
)

target_sources(
    tests
    PRIVATE
    "test/util/utils.cpp"
)

include(Catch)
catch_discover_tests(tests)