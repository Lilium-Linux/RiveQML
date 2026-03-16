# Contributing

## Scope

This project is building a reusable Rive integration for Qt Quick.

Contributions should reinforce these constraints:

- Qt `6.8+` only
- GPU rendering first
- `QQuickWidget` support required
- public QML package shape should stay stable
- internal renderer should remain extensible enough for future software rendering

## Before changing code

Read these first:

- [README.md](./README.md)
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)
- [docs/DEVELOPMENT.md](./docs/DEVELOPMENT.md)
- [docs/INSTALL.md](./docs/INSTALL.md)
- [docs/USAGE.md](./docs/USAGE.md)
- [docs/ROADMAP.md](./docs/ROADMAP.md)

## Contribution guidelines

### 1. Preserve install shape

Do not casually rename:

- `RiveQml::RiveQml`
- `RiveQml 1.0`
- public header names
- top-level QML type names

### 2. Keep backend logic isolated

Do not move backend-specific rendering logic into:

- `RiveItem`
- QML registration code
- public headers

Backend integration belongs behind the internal render backend abstraction.

### 3. Respect `QQuickWidget`

Changes to rendering should be evaluated for both:

- standard Qt Quick windows
- `QQuickWidget`

Do not assume swapchain-only rendering.

### 4. Stay close to official Rive concepts

Prefer terminology and structure that maps cleanly to upstream runtime concepts:

- file
- artboard
- scene
- state machine
- inputs
- view model

Avoid inventing local abstractions that make future upstream integration harder.

### 5. Document significant changes

If you change:

- build layout
- public API
- rendering flow
- dependency strategy

update the corresponding docs in the same change.

## What is welcome now

- real `rive-runtime` integration
- backend bridge work
- package hardening
- tests
- example improvements
- documentation improvements

## What to avoid for now

- broad Qt compatibility work below 6.8
- speculative software renderer work before GPU path is real
- custom feature work that depends on diverging from official Rive runtime behavior
