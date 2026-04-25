add_library(std_module OBJECT)

target_sources(std_module 
    PUBLIC FILE_SET CXX_MODULES FILES 
    std.ixx
    std.compat.ixx
)

target_compile_features(std_module PUBLIC cxx_std_23) 