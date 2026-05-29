# Milestone 5 Record: Camera Transform Path and Grid/Axes Primitive

- **Milestone:** 5
- **Status:** Complete — locally validated by the developer
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

## Local validation status

The developer reported successful completion of the clean build and manual run checks on 2026-05-30 using the intended local VS 2026/Ninja workflow.

Evidence received:

- screenshot of `SfmSandbox` running with the ground grid and green Y axis visible;
- `Sandbox status` panel visible;
- `Renderer: ready` visible;
- positive line vertex count visible;
- renderer messages report shader build, immutable vertex-buffer upload, DSA vertex-array layout and central renderer readiness;
- Milestone 3 shader probe remains passed in the same run;
- Milestone 2 ownership probe remains passed in the same run;
- developer confirmation that camera movement, resize/aspect behavior, legacy regression and sandbox control checks also passed.

Reference build commands:

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
| `EDAF80_Assignment1` | Existing scene launches and interaction still works | Passed — developer confirmed |
| `EDAN35_Assignment2` | Existing deferred-rendering reference launches | Passed — developer confirmed |
| `SfmSandbox` | Window opens and the status panel is visible | Passed — screenshot confirmed |
| `SfmSandbox` | Grid and RGB axes are visible in the viewport | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Milestone 5: Camera transform path and grid/axes primitive` | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows `Renderer: ready` | Passed — screenshot confirmed |
| `SfmSandbox` | Panel shows a positive line vertex count | Passed — screenshot confirmed |
| `SfmSandbox` | Moving/looking with WASD/QE + left mouse changes the grid/axes view | Passed — developer confirmed |
| `SfmSandbox` | Resizing changes framebuffer size and camera aspect readout | Passed — developer confirmed |
| `SfmSandbox` | Milestone 3 shader probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | Milestone 2 ownership probe still passes | Passed — screenshot confirmed |
| `SfmSandbox` | `F2`, `F3`, `F11`, `Esc` still work | Passed — developer confirmed |

## Known limitations at completion

- The renderer still draws generated debug geometry only.
- There is no scene graph, mesh abstraction, camera frustum visualization, point cloud rendering, trajectory rendering or reconstructed mesh rendering yet.
- Grid and axes shaders are still source-string based; file-based shader loading remains deferred.
- The renderer uses a single line pipeline; no render queues, pass graph, materials, depth policy abstraction or draw batching exists yet.

## Completion assessment

| Acceptance criterion | Result |
|---|---|
| Clean Ninja configure/build succeeds in the VS 2026 x64 development environment | Passed — developer confirmed |
| Existing `EDAF80_Assignment1` launches | Passed — developer confirmed |
| Existing `EDAN35_Assignment2` launches | Passed — developer confirmed |
| `SfmSandbox` renders grid and axes through camera transform | Passed — screenshot confirmed |
| Camera movement and resize/aspect path work | Passed — developer confirmed |
| Milestone 3 and Milestone 2 probes remain passing | Passed — screenshot confirmed |
| Completion note records commands and limitations | Passed — this document |

Milestone 5 is complete and may be merged into `feature/sfm-visualization-sandbox`.
