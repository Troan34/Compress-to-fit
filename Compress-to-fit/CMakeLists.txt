# CMakeList.txt : CMake project for Compress-to-fit, include source and define
# project specific logic here.
#


add_executable(Compress-to-fit)
set_target_properties(Compress-to-fit PROPERTIES OUTPUT_NAME "ctf")

target_sources(
    Compress-to-fit
    PRIVATE
    FILE_SET CXX_MODULES FILES
    src/util/utils.cppm
    src/util/cli_parser.cppm
    src/util/file_management.cppm
    src/models/LZ77/lz77.cppm
     
)

target_sources(
    Compress-to-fit
    PRIVATE
    "Compress-to-fit.cpp"
    "src/util/cli_parser.cpp"
    "src/models/LZ77/lz77_impl.cpp"
)



target_compile_features(Compress-to-fit PRIVATE cxx_std_23)
target_link_libraries(Compress-to-fit PRIVATE __CMAKE::CXX23)
target_link_libraries(Compress-to-fit PRIVATE mio::mio)
