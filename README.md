# QtSnapLite

QtSnapLite is a lightweight Qt + VTK desktop viewer for 3D medical volumes.
It provides:

- Axial, coronal, and sagittal slice views with linked crosshair navigation
- A basic VTK volume-rendering panel
- NIfTI loading (`.nii`, `.nii.gz`)
- A built-in synthetic demo volume for quick testing

## Tech Stack

- C++17
- CMake (3.16+)
- Qt 6 Widgets
- VTK (configured for VTK 9.6 in `CMakeLists.txt`)

## Project Structure

```text
src/
  main.cpp
  ui/
    MainWindow.h/.cpp
    SliceView.h/.cpp
  data/
    VolumeData.h/.cpp
  rendering/
    VolumeRenderPlaceholder.h/.cpp
CMakeLists.txt
```

## Build

1. Install Qt 6 and VTK.
2. Update `VTK_DIR` in `CMakeLists.txt` to your local VTK CMake path.
3. Configure and build:

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

## Run

```powershell
.\build\Debug\QtSnapLite.exe
```

## Usage

- Use `File -> Open NIfTI...` to load a volume.
- Use `File -> Open Folder...` or the left `NIfTI Browser` dock to index a folder and pick `.nii` / `.nii.gz` files quickly.
- Use `File -> Load Demo Volume` to load synthetic sample data.
- Move the X/Y/Z sliders in the `Controls` dock to navigate slices.
- Hover any slice view and use the mouse wheel to step through that view's slices.
- Use `View -> Reset View` to center slice positions.

## Notes

- Current rendering/segmentation tooling is intentionally minimal and includes placeholders for future expansion.
