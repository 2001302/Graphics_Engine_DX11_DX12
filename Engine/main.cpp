#include "application.h"

int main() {
    std::unique_ptr<engine::Application> application =
        std::make_unique<engine::Application>();

    if (application->OnStart()) {
        application->Run();
    }

    application->OnStop();

    return 0;
}
