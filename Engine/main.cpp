#include "application.h"

int main() {
    std::unique_ptr<platform::Application> application =
        std::make_unique<platform::Application>();

    if (application->OnStart()) {
        application->Run();
    }

    application->OnStop();

    return 0;
}
