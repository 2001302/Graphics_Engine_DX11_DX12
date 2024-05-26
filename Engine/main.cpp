#include "system.h"


int main()
{
	Engine::System* System;
	bool result;
	
	// Create the system object.
	System = new Engine::System;

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