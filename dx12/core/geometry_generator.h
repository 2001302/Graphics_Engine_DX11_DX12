#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR

#include "graphics_util.h"
#include "model.h"
#include "model_loader.h"
#include "animation_clip.h"
#include <tuple>

namespace core {
class GeometryGenerator {

  public:
    GeometryGenerator(){};
    ~GeometryGenerator(){};

    static std::vector<dx12::MeshData>
    ReadFromFile(std::string basePath, std::string filename,
                 bool revertNormals = false);

    static auto ReadAnimationFromFile(std::string basePath,
                                      std::string filename,
                                      bool revertNormals = false)
        -> std::tuple<std::vector<dx12::MeshData>, AnimationData>;

    static void Normalize(const Vector3 center, const float longestLength,
                          std::vector<dx12::MeshData> &meshes,
                          AnimationData &aniData);

    static dx12::MeshData MakeSquare(const float scale = 1.0f,
                                     const Vector2 texScale = Vector2(1.0f));
    static dx12::MeshData
    MakeSquareGrid(const int numSlices, const int numStacks,
                   const float scale = 1.0f,
                   const Vector2 texScale = Vector2(1.0f));
    static dx12::MeshData MakeBox(const float scale = 1.0f);
    static dx12::MeshData MakeCylinder(const float bottomRadius,
                                       const float topRadius, float height,
                                       int numSlices);
    static dx12::MeshData MakeSphere(const float radius, const int numSlices,
                                     const int numStacks,
                                     const Vector2 texScale = Vector2(1.0f));
    static dx12::MeshData MakeTetrahedron();
    static dx12::MeshData MakeIcosahedron();
    static dx12::MeshData SubdivideToSphere(const float radius,
                                            dx12::MeshData meshData);
    static dx12::MeshData MakeWireBox(const Vector3 center,
                                      const Vector3 extents);
    static dx12::MeshData MakeWireSphere(const Vector3 center,
                                         const float radius);
};
} // namespace engine
#endif
