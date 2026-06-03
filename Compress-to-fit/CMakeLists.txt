add_library(project_logic
        src/data_streamer.cppm)


set_target_properties(project_logic PROPERTIES
    CXX_SCAN_FOR_MODULES ON
    CXX_MODULE_STD ON
)

target_sources(
    project_logic
    PUBLIC
    FILE_SET CXX_MODULES FILES
    "src/util/utils.cppm"
    "src/util/cli_parser.cppm"
    "src/util/file_management.cppm"
    "src/models/LZ77/lz77.cppm"
    "src/util/concurrent_file_buffer.cppm"
)

target_sources(
    project_logic
    PUBLIC
    "src/util/cli_parser.cpp"
    "src/models/LZ77/lz77_impl.cpp"
)

FetchContent_GetProperties(mio SOURCE_DIR mio_SOURCE_DIR)
target_include_directories(project_logic PUBLIC "${mio_SOURCE_DIR}/include")
target_compile_definitions(project_logic PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)

# --- Executable Target: Compress-to-fit ---
add_executable(Compress-to-fit)
set_target_properties(Compress-to-fit PROPERTIES 
    OUTPUT_NAME "ctf"
    CXX_SCAN_FOR_MODULES ON
    CXX_MODULE_STD ON
)
target_link_libraries(Compress-to-fit PRIVATE project_logic)
target_sources(Compress-to-fit PRIVATE "Compress-to-fit.cpp")
target_compile_features(Compress-to-fit PRIVATE cxx_std_23)

# --- Executable Target: tests ---
add_executable(tests)
set_target_properties(tests PROPERTIES
    CXX_SCAN_FOR_MODULES ON
    CXX_MODULE_STD ON
)
target_link_libraries(tests PRIVATE 
    project_logic
    Catch2::Catch2WithMain
    mio
)

target_sources(
    tests
    PRIVATE
    "test/util/utils.cpp"
    "test/benchmarks/file_management.cpp"
)

# --- Platform Specific Flag Settings ---
if(MSVC)
    target_link_options(project_logic PRIVATE "/STACK:10485760")
    target_compile_options(project_logic PUBLIC "/utf-8")
    target_link_options(Compress-to-fit PRIVATE "/STACK:10485760")
    target_compile_options(Compress-to-fit PRIVATE "/utf-8")
elseif(APPLE)
    target_link_options(Compress-to-fit PRIVATE "LINKER:-stack_size,0x1000000")
elseif(WIN32 AND NOT MSVC) # MinGW
    target_link_options(Compress-to-fit PRIVATE "-Wl,--stack,10485760")
else() # Linux
    target_link_options(Compress-to-fit PRIVATE "LINKER:-z,stack-size=10485760")
endif()

include(Catch)
catch_discover_tests(tests)
