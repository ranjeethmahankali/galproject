# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a computational geometry project with C++ core libraries and Python bindings. It consists of three main components:

- **galcore**: Core computational geometry algorithms (static library)
- **galfunc**: Function binding layer with Python integration using pybind11
- **galview**: Interactive 3D viewer application using OpenGL/ImGui for visualizing geometry

The project uses functional programming concepts to compose interactive demos that combine algorithms from galcore with visualization in galview.

## Build System

### Dependencies
- CMake 3.20+, C++20 compiler
- Python 3.12
- vcpkg for dependency management (set VCPKG_PATH environment variable)
- Key libraries: OpenMesh, TBB, OpenGL/GLFW/GLEW, ImGui, pybind11, Boost

### Build Commands
```bash
# Build project (Release by default)
./build.sh [Release|Debug] [target]

# Build specific target
./build.sh Release galview

# Build location: build/bin/Release/ (executables), build/lib/Release/ (libraries)
```

### Testing
```bash
# Run C++ unit tests
Release/galtest

# Run Python script tests
pytest scripts/
```

## Architecture

### Core Libraries
- **galcore**: Pure C++ geometry algorithms, header files in galcore/include/
- **galfunc**: Creates both static library (galfunc) and Python module (pygalfunc.so) from shared object files
- **galview**: Executable that embeds Python interpreter and links to both galcore and galfunc

### Python Integration
- galview initializes Python environment with pygalfunc and pygalview modules
- Demo files (Python scripts in demos/) define algorithm compositions
- Python setup handled automatically during build via setup.py

### Visualization Pipeline
- Shaders in shaders/ (copied to build output automatically)
- Fonts in fonts/ (copied to build output automatically)
- 3D assets in assets/ for testing and demos
- ImGui provides interactive UI elements (sliders, inputs) for demos

### Key Headers
- galcore/include/Types.h: Core type definitions
- galcore/include/Mesh.h: Mesh data structures and algorithms
- galfunc/include/Functions.h: Python binding function declarations
- galview/include/Views.h: Visualization component interfaces

## Development Notes

- Project uses extensive C++ templates and macros (acknowledged by author as experimental)
- Cross-platform support: Linux, macOS, Windows with platform-specific CMake configurations
- Multi-threaded build automatically detects CPU cores
- Compilation commands exported to compile_commands.json for IDE integration
