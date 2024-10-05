#include "engine.h"

void main() {
    std::unique_ptr<graphics::Engine> engine =
        std::make_unique<graphics::Engine>();

    if (engine->Start()) {
        //engine->Prepare();
        engine->Run();
    }

    engine->Stop();
}
