# GraphicsEngine/DX11_DX12_Renderer
![simple](https://github.com/user-attachments/assets/9abc30d8-e34b-430e-8248-8abaa200aa52)

![shuffling](https://github.com/user-attachments/assets/4051dfe9-0e68-4afe-823f-cdbf2df5da52)

![Image](https://github.com/user-attachments/assets/2eaed775-20d3-462f-af89-da286abf11c0)

This engine is based on DirectX 11 and DirectX 12.
- It can create simple 3D models and display them on the screen.
- It can read 3D model files and display them on the screen.
- A physics engine will be added in the future.

# Getting Started
- Requirement
	- Windows OS
	- Visual Studio 2022 installation required
		- Desktop development using C++
		- C++ game development

1. Run third_party/_1.Prepare_Third_Party.bat
2. Build with Visual Studio 2022

# Software Design
## Project Dependencies
![Image](https://github.com/user-attachments/assets/6b556983-145e-43c1-aa5d-41b1ffc43e71)
- Common: A project that includes shared interfaces and data structures, designed to be independent of any specific platform or library.
- GraphicsEngine: A project that manages the rendering pipeline and depends on graphics APIs such as DX12 and DX11. It should be designed for future expansion to support other APIs like Vulkan and Metal.
## Pipeline
![Image](https://github.com/user-attachments/assets/c0b7338a-2b8f-4c0b-a273-13c5f907c703)
## Class Diagram
- GpuCore
![Image](https://github.com/user-attachments/assets/b83c87f0-8253-4a6e-88c3-70b61f90f16b)
- Platform
![Image](https://github.com/user-attachments/assets/3759e3d0-1411-4d2b-a505-b82576f75610)


