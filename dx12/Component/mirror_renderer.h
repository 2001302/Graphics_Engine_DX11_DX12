#ifndef _MIRROR_RENDERER
#define _MIRROR_RENDERER

#include "mesh_renderer.h"
namespace graphics {
class MirrorRenderer : public MeshRenderer {
  public:
    MirrorRenderer(){};
    ~MirrorRenderer(){};

    void SetMirrorPlane(const DirectX::SimpleMath::Plane &plane) {
		mirror_plane = plane;
	}
    void SetMirrorAlpha(float alpha) { 
        mirror_alpha = alpha; }
    DirectX::SimpleMath::Plane GetMirrorPlane() { return mirror_plane; }
    float GetMirrorAlpha() { return mirror_alpha; }

  private:
    DirectX::SimpleMath::Plane mirror_plane;
    float mirror_alpha = 1.0f; // opacity
};
} // namespace graphics
#endif