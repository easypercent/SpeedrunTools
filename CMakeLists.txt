cmake_minimum_required(VERSION 3.17)
project(SpeedrunTools)

set(CMAKE_CXX_STANDARD 17)

get_filename_component(BAKKESMOD_PATH "[HKEY_CURRENT_USER\\Software\\BakkesMod\\AppPath;BakkesModPath]" ABSOLUTE)
#message("BAKKESMOD_PATH = ${BAKKESMOD_PATH}")

add_subdirectory(external/asio)
add_subdirectory(external/cinderblock)
add_subdirectory(external/ocornut/imgui)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")
add_library(SpeedrunTools SHARED ${SOURCE_FILES})

target_precompile_headers(SpeedrunTools PUBLIC src/pch.h)

target_include_directories(
        SpeedrunTools PRIVATE
        "${BAKKESMOD_PATH}/bakkesmodsdk/include"
        "external/asio"
        "external/cinderblock"
        "external/ocornut/imgui"
)

target_link_libraries(
        SpeedrunTools PRIVATE
        "${BAKKESMOD_PATH}/bakkesmodsdk/lib/pluginsdk.lib"
        asio
        RenderingTools
        ImGui
)

target_link_options(SpeedrunTools PUBLIC "/DEBUG")
set_target_properties(SpeedrunTools PROPERTIES COMPILE_OPTIONS "/Zi")
set_target_properties(
        SpeedrunTools PROPERTIES
        COMPILE_PDB_NAME "SpeedrunTools"
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_property(TARGET SpeedrunTools PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)

add_custom_command(
        TARGET SpeedrunTools POST_BUILD
        COMMAND ${BAKKESMOD_PATH}/bakkesmodsdk/bakkesmod-patch.exe $<TARGET_FILE:SpeedrunTools>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running bakkes patch plugin..."
)