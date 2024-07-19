#include "application.h"

int main() {
    std::unique_ptr<dx11::Application> application =
        std::make_unique<dx11::Application>();

    if (application->OnStart()) {
        application->Run();
    }

    application->OnStop();

    return 0;
}
