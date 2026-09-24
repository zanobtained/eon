cmake_minimum_required(VERSION 3.28)

project(Eon LANGUAGES C CXX)

# Set the C/C++ standard requirements.
# * Ensures compiler supports at least C17 and C++17.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# --

# Enable dynamic (shared) library builds.
# * Avoids large executable sizes (maybe).
set(BUILD_SHARED_LIBS ON)
# --

# Define configurations for multi-config generators (e.g., Visual Studio).
# * Has no effect on single-config build systems like Makefiles or Ninja.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
# --

# Define output and asset directories.
# * Used later for copying binaries and resources.
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>/bin")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
# --

# Set output path for compiled executables.
# * CMake places binaries here after specified build directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
# --

# Export compile_commands.json for tooling support.
# * Enables language servers and static analyzers to understand compile flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# --

# Add third-party libraries as subdirectories.
# * Each folder must contain its own CMakeLists.txt file.
add_subdirectory(vendor/SFML)
# --

# Define the main executable and its sources.
# * Header files are listed to improve IDEs visibility to these files.
add_executable(Eon
  source/Utils.cpp
  source/Main.cpp
  source/Game.cpp
  source/Entity.cpp
  source/EntityManager.cpp
  source/DebugOverlay.cpp
  source/Components.cpp

  source/Utils.h
  source/Game.h
  source/Entity.h
  source/EntityManager.h
  source/DebugOverlay.h
  source/Components.h
)
# --

# Link required SFML libraries.
# * CMake will resolves them based on target names.
target_link_libraries(Eon PRIVATE
  SFML::System
  SFML::Window
  SFML::Graphics
)

# Copy SFML DLLs to the output directory after build.
# * Ensures runtime linker can locate shared libraries.
foreach(SFML_LIB System Window Graphics)
  add_custom_command(TARGET Eon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:SFML::${SFML_LIB}>"
    "$<TARGET_FILE_DIR:Eon>"
  )
endforeach()
# --

# Copy game assets to the output directory.
# * Assets must be present at runtime (in the same folder) alongside the executable.
add_custom_command(TARGET Eon POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${ASSETS_DIR}"
  "${OUTPUT_DIR}/../assets"
)
# --

# Set Eon as the startup project in Visual Studio.
# * Required because that heavy bloated shit doesn’t auto-select one.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Eon)
# --
