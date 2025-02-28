cmake_minimum_required(VERSION 3.19)
include(../metamodule-plugin-sdk/plugin.cmake)

project(Moffenzeef VERSION 0.1 DESCRIPTION "Moffenzeef for MetaModule" LANGUAGES C CXX ASM)

add_library(Moffenzeef STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR .)

target_sources(Moffenzeef PRIVATE
    ${SOURCE_DIR}/src/BadIdea9.cpp                    # Note: this file contains init(rack::Plugin*)
    ${SOURCE_DIR}/src/BadIdea1800.cpp
    ${SOURCE_DIR}/src/Bobcat.cpp
    ${SOURCE_DIR}/src/BusMult.cpp
    ${SOURCE_DIR}/src/Count.cpp
    ${SOURCE_DIR}/src/Deviant.cpp
    ${SOURCE_DIR}/src/DialUp.cpp
    ${SOURCE_DIR}/src/GMO.cpp
    ${SOURCE_DIR}/src/Kriket.cpp
    ${SOURCE_DIR}/src/Mito.cpp
    ${SOURCE_DIR}/src/Moffenmix.cpp
    ${SOURCE_DIR}/src/Mongrel.cpp
    ${SOURCE_DIR}/src/Muskrat.cpp
    ${SOURCE_DIR}/src/Simplify.cpp
    ${SOURCE_DIR}/src/plugin.cpp
    ${SOURCE_DIR}/src/plugin.hpp
)

# Add includes and compile options for source repo
target_include_directories(Moffenzeef PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      Moffenzeef                                          # The cmake target name (defined in add_target)
    PLUGIN_NAME     Moffenzeef                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${CMAKE_CURRENT_LIST_DIR}/plugin.json                         # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                  # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins   # Path to where you want the plugin file output
)
