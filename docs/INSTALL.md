# Installation

Repository workflows are validated on macOS and Linux. The current shipping renderer backend is still the macOS CoreGraphics path.

## Requirements

- CMake 3.24+
- Qt 6.8+
- a C++20 compiler
- `premake`

### Package Installation

#### macOS (Homebrew):

```bash
brew install cmake ninja qt premake
```

#### Linux (Arch Linux, pacman/AUR):

```bash
sudo pacman -Syu cmake ninja qt6-base qt6-tools
# premake is available from the AUR:
yay -S premake
```

_If you don't have an AUR helper like yay, install premake manually from the AUR._
_Adjust package names for your distribution as needed (e.g., use `dnf` for Fedora)._

### Qt Path

- **macOS (Apple Silicon):** `/opt/homebrew/opt/qt`
- **macOS (Intel):** `/usr/local/opt/qt`
- **Linux:** Use your distro's Qt install path, e.g., `/usr/lib/qt6` or `/usr/lib/x86_64-linux-gnu/qt6`.

## Runtime Dependency

`RiveQml` builds against the official `rive-runtime` at the pinned revision declared in the project CMake files.

You can provide the runtime in either of these ways:

1. Let `RiveQml` fetch it.
2. Point `RIVEQML_RIVE_RUNTIME_DIR` at an existing checkout.

Useful configure options:

- `RIVEQML_FETCH_RIVE_RUNTIME=ON`
- `RIVEQML_FETCH_RIVE_RUNTIME=OFF`
- `RIVEQML_RIVE_RUNTIME_DIR=/path/to/rive-runtime`
- `RIVEQML_BUILD_TESTS=ON`

## Build

#### Example (macOS):

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt
cmake --build build
ctest --test-dir build --output-on-failure
```

#### Example (Linux):

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/usr/lib/qt6
cmake --build build
ctest --test-dir build --output-on-failure
```

Using a pre-existing local runtime checkout:

#### Using a pre-existing local runtime checkout (macOS):

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt \
  -DRIVEQML_FETCH_RIVE_RUNTIME=OFF \
  -DRIVEQML_RIVE_RUNTIME_DIR=/absolute/path/to/rive-runtime
```

#### Using a pre-existing local runtime checkout (Linux):

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/usr/lib/qt6 \
  -DRIVEQML_FETCH_RIVE_RUNTIME=OFF \
  -DRIVEQML_RIVE_RUNTIME_DIR=/absolute/path/to/rive-runtime
```

## Install

```bash
cmake --install build --prefix /desired/prefix
```

Installed consumer shape:

```cmake
find_package(RiveQml CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE RiveQml::RiveQml)
```

QML import:

```qml
import RiveQml 1.0
```

## Standalone Consumer Example

The recommended consumer validation path is the sibling `RiveQmlExamples` project.

If `RiveQml` is not installed, that project can also consume the sibling source tree directly with `add_subdirectory`.
