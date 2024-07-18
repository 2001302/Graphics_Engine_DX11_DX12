#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR

#include "board_map.h"
#include "direct3D.h"
#include "model.h"
#include "model_loader.h"

namespace dx11 {
class GeometryGenerator {

  public:
    static std::vector<MeshData> ReadFromFile(std::string basePath,
                                         std::string filename,
                                         bool revertNormals = false);

    static MeshData MakeSquare(const float scale = 1.0f,
                               const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeSquareGrid(const int numSlices, const int numStacks,
                                   const float scale = 1.0f,
                                   const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeBox(const float scale = 1.0f);
    static MeshData MakeCylinder(const float bottomRadius,
                                 const float topRadius, float height,
                                 int numSlices);
    static MeshData MakeSphere(const float radius, const int numSlices,
                               const int numStacks,
                               const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeTetrahedron();
    static MeshData MakeIcosahedron();
    static MeshData SubdivideToSphere(const float radius, MeshData meshData);
};
} // namespace dx11
#endif
