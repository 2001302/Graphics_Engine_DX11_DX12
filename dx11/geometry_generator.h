#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR

#include "graphics_util.h"
#include "model.h"
#include "model_loader.h"
#include "animation_clip.h"
#include <tuple>

namespace graphics {
class GeometryGenerator {

  public:
    GeometryGenerator(){};
    ~GeometryGenerator(){};

    static std::vector<MeshData> ReadFromFile(std::string basePath,
                                              std::string filename,
                                              bool revertNormals = false);

    static auto ReadAnimationFromFile(std::string basePath,
                                      std::string filename,
                                      bool revertNormals = false)
        -> std::tuple<std::vector<MeshData>, AnimationData>;

    static void Normalize(const Vector3 center, const float longestLength,
                          std::vector<MeshData> &meshes,
                          AnimationData &aniData);

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
    static MeshData MakeWireBox(const Vector3 center, const Vector3 extents);
    static MeshData MakeWireSphere(const Vector3 center, const float radius);
};
} // namespace engine
#endif
