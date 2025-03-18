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

- Common : 공용 인터페이스 및 자료구조를 포함하는 프로젝트로, 특정 플랫폼이나 라이브러리에 의존성이 없도록 설계한다.
- GraphicsEngine : DX12 및 DX11 등 Graphics API에 의존성을 가지며 Rendering Pipeline을 제어한다. 추후 다른 API으로 확장할 수 있어야한다.(Vulkan, Metal 등)
## Pipeline
## Class Diagram
