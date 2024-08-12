#include "engine.h"

void main() {
    std::unique_ptr<engine::Engine> engine = std::make_unique<engine::Engine>();

    if (engine->Start()) {
        engine->Run();
    }

    engine->Stop();
}
