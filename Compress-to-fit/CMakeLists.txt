add_library(project_logic SHARED)

set(CLANG_TIDY_OPT
        clang-tidy
        --use-color
        "--header-filter=${PROJECT_SOURCE_DIR}/(src|include)/.*"
)

set_target_properties(project_logic PROPERTIES
        CXX_CLANG_TIDY ""
        CXX_SCAN_FOR_MODULES ON
        CXX_MODULE_STD ON
)


target_sources(
    project_logic
    PUBLIC FILE_SET CXX_MODULES FILES
        "src/parser/cli_parser.cppm"
        "src/util/utils.cppm"
        "src/util/containers/file_management.cppm"
        "src/util/containers/file_buffer.cppm"
        "src/util/containers/file_list.cppm"
        "src/util/core_utils.cppm"
        "src/util/containers/containers.cppm"
        "src/util/containers/thread_pool.cppm"
        "src/models/models.cppm"
        "src/models/LZ77/lz77.cppm"
        "src/util/text_style.cppm"
        "src/util/fixed_string.cppm"
    PRIVATE
        "src/models/LZ77/lz77_impl.cpp"
)

target_sources(
    project_logic
    PUBLIC
        "src/parser/cli_parser.cpp"
        "src/common/comp_options.hpp"
        "src/common/error_warn_print.hpp"
)

#set_target_properties(project_logic PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_OPT})



FetchContent_GetProperties(mio SOURCE_DIR mio_SOURCE_DIR)
target_include_directories(project_logic PUBLIC "${mio_SOURCE_DIR}/include")
target_compile_definitions(project_logic PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)



#add the main executable
add_executable(Compress-to-fit)
set_target_properties(Compress-to-fit PROPERTIES 
    OUTPUT_NAME "ctf"
    CXX_SCAN_FOR_MODULES ON
    CXX_MODULE_STD ON
    CXX_CLANG_TIDY ""
)
target_link_libraries(Compress-to-fit PRIVATE project_logic)

target_sources(Compress-to-fit PRIVATE "Compress-to-fit.cpp")

target_compile_features(Compress-to-fit PRIVATE cxx_std_23)
set_target_properties(Compress-to-fit PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_OPT})


#add the gui executable
add_executable(Compress-to-fit-gui)
set_target_properties(Compress-to-fit-gui PROPERTIES
        OUTPUT_NAME "ctf_gui"
        CXX_SCAN_FOR_MODULES ON
        CXX_MODULE_STD ON
        CXX_CLANG_TIDY ""
)
target_link_libraries(Compress-to-fit-gui PRIVATE project_logic)

target_link_libraries(
        Compress-to-fit-gui
        PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Widgets
)

target_sources(Compress-to-fit-gui PRIVATE "Compress-to-fit-gui.cpp")

target_compile_features(Compress-to-fit-gui PRIVATE cxx_std_23)
set_target_properties(Compress-to-fit-gui PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_OPT})
qt_add_qml_module(Compress-to-fit-gui
        URI CompressToFit
        VERSION 1.0

        QML_FILES
            qml/Main.qml
            qml/common/FileEntry.qml
            qml/common/InteractiveDropArea.qml

        RESOURCES
            qml/icons/docs_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg
            qml/icons/folder_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg
            qml/icons/photo_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg
            qml/icons/files_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24.svg
)

target_sources(
    Compress-to-fit-gui
    PRIVATE
        "src/gui/interface.hpp"
        "src/gui/interface.cpp"
        "src/gui/fs_helper.hpp"
        "src/gui/fs_helper.cpp"
)

#add the tests executable
add_executable(tests)
set_target_properties(tests PROPERTIES
        CXX_SCAN_FOR_MODULES ON
        CXX_MODULE_STD ON
        CXX_CLANG_TIDY ""
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

set_target_properties(tests PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_OPT})


#platform dependent flags
if(MSVC)
    target_link_options(project_logic PUBLIC "/STACK:10485760")
    target_compile_options(project_logic PUBLIC "/utf-8")
elseif(APPLE)
    target_link_options(project_logic PUBLIC "LINKER:-stack_size,0x1000000")
elseif(WIN32 AND NOT MSVC) # MinGW
    target_link_options(project_logic PUBLIC "-Wl,--stack,10485760")
else() # Linux
    target_link_options(project_logic PUBLIC "LINKER:-z,stack-size=10485760")
endif()

include(Catch)
catch_discover_tests(tests)

if(TARGET __cmake_cxx_std_23)
    set_property(
            TARGET __cmake_cxx_std_23
            PROPERTY CXX_CLANG_TIDY ""
    )
endif()