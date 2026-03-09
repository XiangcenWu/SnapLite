# SnapLite

SnapLite is a lightweight Qt + VTK desktop viewer for 3D medical volumes.
It provides:

- Axial, coronal, and sagittal slice views with linked crosshair navigation
- A basic VTK volume-rendering panel
- NIfTI loading (`.nii`, `.nii.gz`)
- A built-in synthetic demo volume for quick testing

## Tech Stack

- C++17
- CMake (3.21+)
- Qt 6 Widgets
- VTK (tested with VTK 9.6)

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
2. Point CMake at your Qt / VTK package locations.
3. Configure and build:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64;C:/VTK-9.6.0/build"
cmake --build build --config Debug
```

## Run

```powershell
.\build\Debug\SnapLite.exe
```

## Package for GitHub Releases on Windows

Packaging is configured for Windows only and produces a ZIP archive that is ready
to upload to a GitHub Release.

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64;C:/VTK-9.6.0/build"
cmake --build build --config Release --target package
```

The generated archive will be placed in `build/` with a name like:

```text
SnapLite-0.1.0-windows-x64.zip
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
