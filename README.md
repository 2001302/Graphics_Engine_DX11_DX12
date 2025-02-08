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

# Structure

![Image](https://github.com/user-attachments/assets/74e371e0-1f8b-4e6d-adc8-b9322630912b)

# BehaviorTree

![Image](https://github.com/user-attachments/assets/c24d9c90-4668-4c82-8b1c-a3a1256cebb0)

- Basically, the process follows the order of data block bind-build-run.
- Individual functionalities are managed by separate Nodes.
- The execution of a Node can be determined at runtime.
- Nodes should minimize the use of new and reuse resources from the DataBlock whenever possible.
- If a Node fails, it logs an error and may escalate the failure to the parent Node.
