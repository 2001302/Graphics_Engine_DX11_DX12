#include "application.h"


int main()
{
	std::unique_ptr<Engine::Application> application = std::make_unique<Engine::Application>();

	// Initialize and run the system object.
	if (application->OnStart())
	{
		application->Run();
	}

	application->OnStop();

	return 0;
}