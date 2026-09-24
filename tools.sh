#!/bin/sh

# Define few things to run logic.
# * The specified executable name must match CMakeLists.txt's executable name.
EXECUTABLE_NAME="Eon"
BUILD_DIRECTORY="build"

BUILD_GENERATOR=""
C_COMPILER=""
CPP_COMPILER=""

BUILD_CONFIG=""
RUN_CONFIG=""

DEFAULT_COMMAND=""
GENERATE_COMMAND="generate"
BUILD_COMMAND="build"
RUN_COMMAND="run"

GENERATOR_MAKEFILE_MINGW_PARAM="-make_mingw"
GENERATOR_MAKEFILE_UNIX_PARAM="-make_unix"
GENERATOR_NINJA_PARAM="-ninja"
GENERATOR_NINJA_MULTI_CONFIG_PARAM="-ninja_mc"
GENERATOR_VISUAL_STUDIO_14_PARAM="-vs14"
GENERATOR_VISUAL_STUDIO_15_PARAM="-vs15"
GENERATOR_VISUAL_STUDIO_16_PARAM="-vs16"
GENERATOR_VISUAL_STUDIO_17_PARAM="-vs17"
GENERATOR_XCODE_PARAM="-xcode"

COMPILER_CLANG_PARAM="-clang"
COMPILER_GCC_PARAM="-gcc"

DEBUG_PARAM="-deb"
RELEASE_PARAM="-rel"
# --

# Print this script's usability.
# * Show information about how to use the script.
print_usage() {
  echo ""
  echo "usage: $0 [ generate -<generator> -<compiler> | build -(deb/rel) | run -(deb/rel) ]"
  echo "-----------------------------------------------------------------------------------------"
  echo "  generate  : generate build files with cmake"
  echo "              (defaults to detected generator)"
  echo "              (compiler will fallback to generator if one do not support)"
  echo "              current supported generator:"
  echo "                *MinGW Makefiles (-make_mingw) [Windows]"
  echo "                *Unix Makefiles (-make_unix) [Linux/MacOS]"
  echo "                *Ninja (-ninja) [Windows/Linux/MacOS]"
  echo "                *Ninja Multi-Config (-ninja_mc) [Windows/Linux/MacOS]"
  echo "                *Visual Studio 14 2015 (-vs14) [Windows]"
  echo "                *Visual Studio 15 2017 (-vs15) [Windows]"
  echo "                *Visual Studio 16 2019 (-vs16) [Windows]"
  echo "                *Visual Studio 17 2022 (-vs17) [Windows]"
  echo "                *Xcode (-xcode) [MacOS]"
  echo "              current supported compiler:"
  echo "                *clang and clang++ (-clang)"
  echo "                *gcc and g++ (-gcc)"
  echo "  build     : build files with specified generator"
  echo "              (defaults to debug if not specified)"
  echo "  run       : run executable binary for specified configuration"
  echo "              (defaults to debug if not specified)"
  echo "-----------------------------------------------------------------------------------------"
  echo ""
}
# --

# Guard for command 'generate'.
# * Set generator to be used by CMake based on supported generator flag. 
# * Specific compiler also can be supported if one is supported flag.
# * If there's something wrong, print_usage function will be called.
if [ "$1" = "$GENERATE_COMMAND" ]; then
  if [ "$2" = "$GENERATOR_MAKEFILE_MINGW_PARAM" ]; then
    BUILD_GENERATOR="MinGW Makefiles"
  elif [ "$2" = "$GENERATOR_MAKEFILE_UNIX_PARAM" ]; then
    BUILD_GENERATOR="Unix Makefiles"
  elif [ "$2" = "$GENERATOR_NINJA_PARAM" ]; then
    BUILD_GENERATOR="Ninja"
  elif [ "$2" = "$GENERATOR_NINJA_MULTI_CONFIG_PARAM" ]; then
    BUILD_GENERATOR="Ninja Multi-Config"
  elif [ "$2" = "$GENERATOR_VISUAL_STUDIO_14_PARAM" ]; then
    BUILD_GENERATOR="Visual Studio 14 2015"
  elif [ "$2" = "$GENERATOR_VISUAL_STUDIO_15_PARAM" ]; then
    BUILD_GENERATOR="Visual Studio 15 2017"
  elif [ "$2" = "$GENERATOR_VISUAL_STUDIO_16_PARAM" ]; then
    BUILD_GENERATOR="Visual Studio 16 2019"
  elif [ "$2" = "$GENERATOR_VISUAL_STUDIO_17_PARAM" ]; then
    BUILD_GENERATOR="Visual Studio 17 2022"
  elif [ "$2" = "$GENERATOR_XCODE_PARAM" ]; then
    BUILD_GENERATOR="Xcode"
  elif [ -z "$2" ]; then
    BUILD_GENERATOR=""
  else
    echo ""
    echo "Unknown or not supported generator. See below:"
    print_usage
    exit 1
  fi

  if [ "$3" = "$COMPILER_CLANG_PARAM" ]; then
    C_COMPILER="clang"
    CPP_COMPILER="clang++"
  elif [ "$3" = "$COMPILER_GCC_PARAM" ]; then
    C_COMPILER="gcc"
    CPP_COMPILER="g++"
  elif [ -z "$3" ]; then
    C_COMPILER=""
    CPP_COMPILER=""
  else
    echo ""
    echo "Unknown or not supported compiler. See below:"
    print_usage
    exit 1
  fi
fi
# --

# Guard for comand 'build'.
# * Set build configuration (Debug/Release).
# * If a generator doesn't support multi-config will fallback to generator's default.
# * If there's something wrong, print_usage function will be called.
if [ "$1" = "$BUILD_COMMAND" ]; then
  if [ "$2" = "$DEBUG_PARAM" ]; then
    BUILD_CONFIG="Debug"
  elif [ "$2" = "$RELEASE_PARAM" ]; then
    BUILD_CONFIG="Release"
  elif [ -z "$2" ]; then
    BUILD_CONFIG="Debug"
  else
    echo ""
    echo "Unknown build type. See below:"
    print_usage
    exit 1
  fi
fi
# --

# Guard for command 'run'.
# * Set type of executable will be run (Debug/Release) from its binary directory.
# * If a generator doesn't support multi-config will fallback to generator's default directory.
# * If there's something wrong, print_usage function will be called.
if [ "$1" = "$RUN_COMMAND" ]; then
  if [ "$2" = "$DEBUG_PARAM" ]; then
    RUN_CONFIG="Debug"
  elif [ "$2" = "$RELEASE_PARAM" ]; then
    RUN_CONFIG="Release"
  elif [ -z "$2" ]; then
    RUN_CONFIG="Debug"
  else
    echo ""
    echo "Unknown executable type. See below:"
    print_usage
    exit 1
  fi
fi
# --

# Generate project files with CMake.
# * Generating with this script meaning the build folder will always be named 'build'.
generate() {
  if [ -n "$BUILD_GENERATOR" ]; then
    if [ -n "$C_COMPILER" ] && [ -n "$CPP_COMPILER" ]; then
      exec cmake -B "$BUILD_DIRECTORY" \
                 -G "$BUILD_GENERATOR" \
                 -D CMAKE_C_COMPILER="$C_COMPILER" \
                 -D CMAKE_CXX_COMPILER="$CPP_COMPILER"
    else
      exec cmake -B "$BUILD_DIRECTORY" \
                 -G "$BUILD_GENERATOR"
    fi
  else
    exec cmake -B "$BUILD_DIRECTORY"
  fi
}
# --

# Build to binaries with CMake.
# * Using multi-core compilation process.
build() {
  exec cmake --build "$BUILD_DIRECTORY" --config "$BUILD_CONFIG" --parallel $(nproc)
}
# --

# Runs executable from binaries directory.
# * Assuming that the build directory match default one which is 'build' so it can work.
run() {
  if [ -x "$BUILD_DIRECTORY/$RUN_CONFIG/bin/$EXECUTABLE_NAME" ]; then
    exec "./$BUILD_DIRECTORY/$RUN_CONFIG/bin/$EXECUTABLE_NAME"
  elif [ -x "$BUILD_DIRECTORY/bin/$EXECUTABLE_NAME" ]; then
    exec "./$BUILD_DIRECTORY/bin/$EXECUTABLE_NAME"
  else
    echo ""
    echo "Executable not found, did you build it first?"
    exit 1
  fi
}
# --

# Enable commands/flags based on arguments provided.
# * Script invoke will call function according to provided arguments.
case "$1" in
  $DEFAULT_COMMAND)
    print_usage
    exit 1
    ;;
  $GENERATE_COMMAND)
    generate
    ;;
  $BUILD_COMMAND)
    build
    ;;
  $RUN_COMMAND)
    run
    ;;
  *)
    echo ""
    echo "Invalid parameter. See below:"
    print_usage
    exit 1
    ;;
esac
# --
