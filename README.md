### how to get this to work
- if you're on windows:
  1. [install vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows)
  2. [get cmake to work with vcpkg](https://github.com/microsoft/vcpkg#using-vcpkg-with-cmake)
  3. `vcpkg install ...` these:
     1. glfw3:x64-windows
     2. glm:x64-windows
  4. [get the vulkan sdk](https://www.lunarg.com/vulkan-sdk/)
- if you're on ubuntu:
  1. `sudo apt install ...` these:
     1. vulkan-tools
     2. libvulkan-dev
     3. vulkan-validationlayers-dev
     4. spirv-tools
     5. libglfw3-dev
     6. libglm-dev