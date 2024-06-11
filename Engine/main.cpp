#include "Platform.h"


int main()
{
	Engine::Platform* Platform;
	bool result;

	// Create the system object.
	Platform = new Engine::Platform;

	// Initialize and run the system object.
	if (Platform->Initialize())
	{
		Platform->Run();
	}

	delete Platform;
	Platform = 0;

	return 0;
}