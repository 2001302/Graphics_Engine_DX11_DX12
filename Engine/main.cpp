#include "systemclass.h"


int main()
{
	Engine::SystemClass* System;
	bool result;
	
	// Create the system object.
	System = new Engine::SystemClass;

	// Initialize and run the system object.
	result = System->Initialize();
	if(result)
	{
		System->Run();
	}

	// Shutdown and release the system object.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}