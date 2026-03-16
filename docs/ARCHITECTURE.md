# Architecture

## Layers

### Public QML Layer

Files in `include/RiveQml/` expose the stable consumer-facing API:

- `RiveItem`
- `RiveFile`
- `RiveInputMap`
- `RiveViewModelAdapter`
- `RiveAssetProvider`

### File Load Layer

`RuntimeBridge` loads `.riv` bytes through the official `rive-runtime`, imports the file, and exposes file metadata back to `RiveFile`.

Responsibilities:

- source resolution
- file import
- asset lookup root selection
- metadata extraction

### Per-Item Runtime Layer

`RiveItem` owns the mutable runtime state for one visual instance:

- artboard instance
- selected animation or state machine instance
- current input values
- current bound default view-model instance
- playback clock

This keeps playback state isolated even when multiple items point at the same `RiveFile`.

### Render Layer

The current renderer path is:

1. upstream `CGFactory`
2. upstream `CGRenderer`
3. bitmap render into a `QImage`
4. presentation through `QQuickPaintedItem`

This was chosen to ship a real, supported render path quickly on macOS without keeping the old fake scenegraph backend alive.

## Why `QQuickPaintedItem`

The previous placeholder architecture was built around a custom `QSGRenderNode`, but there was no real backend behind it.

The current shipped backend uses `QQuickPaintedItem` because it gives:

- a real render path today
- compatibility with standard Qt Quick usage
- compatibility with `QQuickWidget`-style hosting on macOS without requiring Qt backend-specific GPU glue first

## Current Tradeoff

The CoreGraphics path is a production implementation, but it is not the final performance target.

The long-term architecture still points toward native GPU backends once they are wired cleanly into Qt.
