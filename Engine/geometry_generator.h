#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR

#include "board_map.h"
#include "cube_map.h"
#include "graphics_manager.h"
#include "model.h"
#include "model_loader.h"

namespace engine {
class GeometryGenerator {

  public:
    GeometryGenerator(){};
    ~GeometryGenerator(){};

    static CubeMap *MakeBox(CubeMap *gameObject);
    static CubeMap *MakeSphere(CubeMap *gameObject, const float radius,
                               const int numSlices, const int numStacks);
    static Model *MakeSphere(Model *gameObject, const float radius,
                             const int numSlices, const int numStacks);
    static Model *MakeBox(Model *gameObject);
    static Model *MakeCylinder(Model *gameObject, const float bottomRadius,
                               const float topRadius, float height,
                               int numSlices);
    static BoardMap *MakeSquare(BoardMap *board, const float scale = 1.0f,
                                const Vector2 texScale = Vector2(1.0f));
    static Model *ReadFromFile(Model *gameObject, std::string basePath,
                               std::string filename);
    static Model *MakeSquareGrid(Model *ground, const int numSlices,
                                  const int numStacks, const float scale,
                                  const Vector2 texScale);
};
} // namespace dx11
#endif
