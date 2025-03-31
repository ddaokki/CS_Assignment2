# cg_code
# Ray Tracing Assignment

## ✅ Description
A basic ray tracer using OpenGL and C++ (Visual Studio).

## 🛠️ Compilation & Run Instructions

1. Open `RayTracingProject.sln` with Visual Studio.
2. Build the solution (`Ctrl+Shift+B`)
3. Run the program (`F5`)
4. Make sure to install GLEW and GLFW if not installed.

## 📦 Requirements
- Visual Studio 2019 or later
- GLEW
- GLFW
- GLM

## 🖼️ Screenshots
See `/Result`



## 🧠 Features by Question

| Part | Features                             |
|------|--------------------------------------|
| Q1   | Phong shading (ambient, diffuse, specular), **no shadows** |
| Q2   | Adds **hard shadows** using shadow rays |
| Q3   | Adds **gamma correction** and **64x antialiasing** |

---

## 🎯 Key Code Components

- **Ray**: Casts from camera into the scene.
- **Surface**: Abstract class (Sphere, Plane inherit from this).
- **Phong Shading**: Calculates ambient + diffuse + specular using light direction and surface normal.
- **Shadow Ray**: For occlusion checks between surface and light.
- **Gamma Correction**: Applied at the end of shading using γ = 2.2
- **Antialiasing**: 64 random samples per pixel using a box filter.


