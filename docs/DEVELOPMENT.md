# Development

Repository workflows are validated on macOS and Linux. Current shipping raster backends are CoreGraphics on macOS and Skia raster on Linux.

## Local Workflow

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt

cmake --build build
ctest --test-dir build --output-on-failure
```

Use the matching Qt prefix for your platform, for example `/opt/homebrew/opt/qt` on macOS or `/usr/lib/qt6` on Linux.

On Linux, the first renderer-enabled build also bootstraps the pinned Skia checkout and can take noticeably longer than incremental builds.

If you already have a local runtime checkout:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt \
  -DRIVEQML_FETCH_RIVE_RUNTIME=OFF \
  -DRIVEQML_RIVE_RUNTIME_DIR=/absolute/path/to/rive-runtime
```

## Key Rules

- keep the public QML API stable unless there is a strong reason to break it
- prefer upstream runtime features over local reimplementation
- keep the module buildable as a standalone package consumer dependency
- keep standalone examples outside this repository

## What To Test

At minimum, validate:

- `RiveFile` loads a real `.riv` file
- a `RiveItem` renders without placeholder output
- state machine inputs update the runtime scene
- default view-model values flow from QML into the runtime instance
- standalone consumer configuration still works

## Current Test Coverage

The repository ships a smoke test for real file loading.

When touching runtime integration or rendering, also rebuild the sibling `RiveQmlExamples` project, run the brightness-control example, and rerun the `RiveQmlRiveItemRenderTest` smoke coverage.
