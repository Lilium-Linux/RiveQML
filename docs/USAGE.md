# Usage

## Core Types

### `RiveFile`

`RiveFile` owns source loading and file metadata:

- `source`
- `status`
- `graphicsApi`
- `errorString`
- `artboards`
- `animations`
- `stateMachines`

### `RiveItem`

`RiveItem` owns the per-view runtime state:

- selected artboard
- selected animation or state machine
- playback state
- current input values
- current bound default view-model instance
- rendering into the Qt Quick scene

Important properties:

- `source`
- `document`
- `artboard`
- `animation`
- `stateMachine`
- `fitMode`
- `alignment`
- `autoplay`
- `paused`
- `speed`
- `interactive`
- `inputMap`
- `viewModel`

Important invokables:

- `setNumber(name, value)`
- `setBool(name, value)`
- `fireTrigger(name)`
- `setTextRun(name, value)`
- `bindViewModel(adapter)`
- `reload()`

## State Machine Inputs

```qml
RiveInputMap {
    id: inputs
}

RiveItem {
    stateMachine: "Main"
    inputMap: inputs
}

Slider {
    from: 0
    to: 100
    onValueChanged: inputs.setNumber("progress", value)
}
```

`fireTrigger(name)` emits a one-shot trigger pulse instead of storing a permanent boolean input.

## View Model Binding

`RiveItem` binds the default view-model instance attached to the selected artboard.

```qml
RiveViewModelAdapter {
    id: viewModel
}

RiveItem {
    source: "qrc:/assets/brightnessControl.riv"
    viewModel: viewModel
}

Slider {
    from: 0
    to: 100
    onValueChanged: viewModel.setValue("value", value)
}
```

Current scope:

- pushes values from QML into the default runtime view-model instance
- supports number, boolean, string, trigger, enum, and color updates

## Shared Documents

Use `RiveFile` when multiple items should share the same loaded source and metadata.

```qml
RiveFile {
    id: file
    source: "qrc:/assets/example.riv"
}

Row {
    RiveItem { document: file }
    RiveItem { document: file; stateMachine: "Secondary" }
}
```

## Asset Resolution

Default external asset lookup is source-relative.

`RiveAssetProvider` can override the asset root when `RiveItem` is using its internal `RiveFile`.

## Current Limitations

- tested on macOS and Linux, but the current shipping renderer backend is the macOS CoreGraphics path
- renderer backend is CoreGraphics-based, not GPU-native yet
- view-model sync is currently write-to-runtime from QML; it is not a full two-way observer bridge
