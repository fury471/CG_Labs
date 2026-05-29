# Milestone 5 Record: Camera Transform Path and Grid/Axes Primitive

- **Milestone:** 5
- **Status:** Implementation ready for local validation
- **Date:** 2026-05-30
- **Development branch:** `feature/sfm-m5-camera-grid-axes`
- **Integration branch:** `feature/sfm-visualization-sandbox`

## Goal

Turn the sandbox renderer from a clip-space triangle demo into a small 3D visualization viewport. The renderer should draw world-space reference geometry using the camera's world-to-clip transform, while the application remains responsible for frame lifecycle, input, camera updates and UI.

## Implemented source boundaries

```text
src/sandbox/gfx/
  Renderer.hpp/.cpp             World-space grid/axes renderer using camera transform
src/apps/SfmSandbox/main.cpp    Camera aspect update and world-to-clip submission
```

No new scene layer is introduced yet. The renderer still owns a single debug visualization primitive, but that primitive is now transformed by the camera path that future SfM visualization data will use.

## Renderer behavior

`Renderer` now owns and draws:

- an XZ-plane ground grid;
- a red X axis;
- a green Y axis;
- a blue Z axis;
- a cached `u_world_to_clip` uniform location;
- one immutable vertex buffer and one DSA-configured vertex array.

The renderer no longer draws a hard-coded clip-space triangle. Its `render()` method now takes:

```cpp
void render(glm::mat4 const& world_to_clip) const noexcept;
```

The application calls it with:

```cpp
renderer.render(camera.GetWorldToClipMatrix());
```

## Camera path

`SfmSandbox` now uses the existing Bonobo `FPSCameraf` as the first camera source for the sandbox renderer.

Per frame:

- input updates the camera;
- framebuffer size is queried;
- camera aspect is updated from the actual framebuffer dimensions;
- the renderer receives `camera.GetWorldToClipMatrix()`.

The status panel also displays the camera aspect so resize behavior is visible during manual validation.

## Architectural compliance check

| Rule | Implementation assessment |
|---|---|
| Renderer receives a camera transform instead of drawing in clip space | Implemented through `Renderer::render(glm::mat4 const&)` |
| First 3D reference primitive exists | Implemented as grid/axes line geometry |
| Uniform update uses cached binding path | `u_world_to_clip` is cached during renderer initialization |
| Application owns camera/input, renderer owns drawing | Preserved in `SfmSandbox` and `Renderer` responsibilities |
| Milestone 3 and 2 probes remain visible | `SfmSandbox` still displays both regression probes |
| Legacy assignments remain untouched | No `src/EDAF80/` or `src/EDAN35/` source file is modified |

## Local validation required

From the Visual Studio 2026 x64 Developer Command Prompt:

```bat
cd /d E:\Lund\S1P1\computerGraphics\labs\lab\CG_Labs
git fetch origin
git switch feature/sfm-m5-camera-grid-axes
git pull
rmdir /s /q build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Manual launch checklist:

| Executable | Required observation | Result |
|---|---|---|
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Pending |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Pending |
| `SfmSandbox` | Window opens and the status panel is visible | Pending |
| `SfmSandbox` | Grid and RGB axes are visible in the viewport | Pending |
| `SfmSandbox` | Panel shows `Milestone 5: Camera transform path and grid/axes primitive` | Pending |
| `SfmSandbox` | Panel shows `Renderer: ready` | Pending |
| `SfmSandbox` | Panel shows a positive line vertex count | Pending |
| `SfmSandbox` | Moving/looking with WASD/QE + left mouse changes the grid/axes view | Pending |
| `SfmSandbox` | Resizing changes framebuffer size and camera aspect readout | Pending |
| `SfmSandbox` | Milestone 3 shader probe still passes | Pending |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Pending |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc` still work | Pending |

## Known limitations at this checkpoint

- The renderer still draws generated debug geometry only.
- There is no scene graph, mesh abstraction, camera frustum visualization, point cloud rendering, trajectory rendering or reconstructed mesh rendering yet.
- Grid and axes shaders are still source-string based; file-based shader loading remains deferred.
- The renderer uses a single line pipeline; no render queues, pass graph, materials, depth policy abstraction or draw batching exists yet.

## Completion gate

Milestone 5 is complete only after the local validation checklist passes and this document is updated from `Pending` to confirmed results.
