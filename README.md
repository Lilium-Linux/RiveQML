# RiveQml

`RiveQml` is a Qt 6 QML module for rendering `.riv` files with the official `rive-runtime`.

## Status

Current production path:

- real `.riv` loading through the pinned upstream runtime
- real artboard, animation, and state machine instancing per `RiveItem`
- state machine input updates through `RiveInputMap`
- default view-model binding through `RiveViewModelAdapter`
- tested in repository workflows on macOS and Linux
- software-backed rendering on macOS through the upstream CoreGraphics renderer
- Linux builds currently use a no-op renderer backend, so package integration and example builds work there but raster output does not yet
- build-tree and install-tree CMake package exports
- standalone consumer examples live outside this repository in `RiveQmlExamples`

Current platform scope:

- tested on macOS and Linux
- current shipping raster renderer backend is the macOS CoreGraphics path
- non-Apple builds currently load documents and expose the API surface, but they do not render frames yet

## Public QML Types

- `RiveItem`
- `RiveFile`
- `RiveInputMap`
- `RiveViewModelAdapter`
- `RiveAssetProvider`

Example:

```qml
import QtQuick
import RiveQml 1.0

RiveInputMap {
    id: inputs
}

RiveViewModelAdapter {
    id: viewModel
}

RiveItem {
    width: 480
    height: 320
    source: "qrc:/assets/example.riv"
    stateMachine: "Main"
    inputMap: inputs
    viewModel: viewModel
}
```

## Build

Requirements:

- CMake 3.24+
- Qt 6.8+
- C++20 toolchain
- `premake`
- macOS for the current shipping raster renderer backend

Linux builds currently validate package consumption and example integration, but they do not render frames yet.

Typical setup on Apple Silicon:

```bash
brew install cmake ninja qt premake

cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt

cmake --build build
ctest --test-dir build --output-on-failure
```

`RiveQml` can either use a local `rive-runtime` checkout or fetch the pinned upstream revision during configure.

Key options:

- `RIVEQML_FETCH_RIVE_RUNTIME`
- `RIVEQML_RIVE_RUNTIME_DIR`
- `RIVEQML_BUILD_TESTS`

## Examples

This repository no longer ships in-tree example applications.

Use the standalone consumer project in the sibling `RiveQmlExamples` checkout to validate package consumption and end-user integration.

## Documentation

- [Installation](./docs/INSTALL.md)
- [Usage](./docs/USAGE.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Development](./docs/DEVELOPMENT.md)
- [Roadmap](./docs/ROADMAP.md)
