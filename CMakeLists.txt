# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 4.0.3 FATAL_ERROR)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

project ("Compress-to-fit" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)

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



