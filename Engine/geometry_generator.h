#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR

#include "cube_map.h"
#include "direct3D.h"
#include "game_object.h"
#include "model_loader.h"
#include "post_process.h"

namespace Engine {
class GeometryGenerator {

  public:
    GeometryGenerator(){};
    ~GeometryGenerator(){};

    static CubeMap *MakeBox(CubeMap *gameObject);
    static CubeMap *MakeSphere(CubeMap *gameObject, const float radius,
                               const int numSlices, const int numStacks);
    static GameObject *MakeSphere(GameObject *gameObject, const float radius,
                                  const int numSlices, const int numStacks);
    static GameObject *MakeBox(GameObject *gameObject);
    static GameObject *MakeCylinder(GameObject *gameObject,
                                    const float bottomRadius,
                                    const float topRadius, float height,
                                    int numSlices);
    static PostProcess *MakeSquare(PostProcess *postProcess,
                                   const float scale = 1.0f,
                                   const Vector2 texScale = Vector2(1.0f));
    static GameObject *ReadFromFile(GameObject *gameObject,
                                    std::string basePath, std::string filename);
};
} // namespace Engine
#endif