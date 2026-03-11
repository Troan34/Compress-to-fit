# CMakeList.txt : CMake project for Compress-to-fit, include source and define
# project specific logic here.
#


add_executable(Compress-to-fit)

target_sources(
    Compress-to-fit
    PRIVATE
    "Compress-to-fit.cpp"
    "src/util/cli_parser.cpp"
    "src/util/file_management_impl.cpp"  
)

target_sources(
    Compress-to-fit
    PRIVATE
    FILE_SET CXX_MODULES FILES
    src/util/utils.cppm
    src/util/file_management.cppm
    src/util/cli_parser.cppm
)
target_compile_features(Compress-to-fit PRIVATE cxx_std_23)
target_link_libraries(Compress-to-fit PRIVATE __CMAKE::CXX23)


