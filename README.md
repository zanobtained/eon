# Eon

This repository was created for **learning purposes** only. Everything could be a bad idea.

---

## Installation

- **Prerequisites**
  - [CMake](https://cmake.org/)
  - C++ Compiler (eg. [Clang](https://llvm.org/), [GCC](https://gcc.gnu.org/), [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/))
  - [SFML necessary dependencies](https://www.sfml-dev.org/tutorials/3.0/getting-started/build-from-source/#installing-dependencies) (for Linux users)

- **Clone Repositories**
  - ```bash
    git clone https://github.com/jeanersten/Eon.git && cd Eon
    ```

- **Initialize Submodule**
  - ```bash
    git submodule update --init --recursive
    ```

- **Building**
  - **With tools.sh**: In order to generate build files you can use the tools.sh script
    if you have shell script invoker. In Linux this should be supported by default,
    in Windows you can use Git shell script invoker if it's listed on the environment
    variable path, I don't know about MacOS. You can then run:
    ```bash
    sh tools.sh
    ```
    This will show of what the script can do. It's useful if you don't
    use IDE, you can generate/build/run via command line.

  - **Without tools.sh**: If you don't have shell script invoker you can run:
    ```bash
    cmake -G "generator (eg. Unix Makefiles/Ninja)" -B "build directory"
    ```
    This will generate build files of your preferred generator into specified build directory.
