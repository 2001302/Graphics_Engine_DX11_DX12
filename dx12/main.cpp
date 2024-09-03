#include "core/engine.h"


void main() {
    std::unique_ptr<core::Engine> engine = std::make_unique<core::Engine>();

    if (engine->Start()) {
        engine->Run();
    }

    engine->Stop();
}
