#ifndef _RESOURCEHELPER
#define _RESOURCEHELPER

#include "direct3D.h"
#include "model.h"

namespace Engine {

/// <summary>
/// Resource Load/Save/Convert 등 작업을 수행하는 class.
/// NOTE : member 변수를 가지지 말 것.
/// </summary>
class ResourceHelper {

  public:
    ResourceHelper(){};
    ~ResourceHelper(){};

    static void
    CreateTexture(const std::string filename, ComPtr<ID3D11Texture2D> &texture,
                  ComPtr<ID3D11ShaderResourceView> &textureResourceView,
                  bool usSRGB = false);

    static void
    CreateTextureArray(const std::vector<std::string> filenames,
                       ComPtr<ID3D11Texture2D> &texture,
                       ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static Model *ImportModel(Model *gameObject,
                                   const char *filename);
    static void ReadModelData(Model *gameObject, const aiScene *scene,
                              aiNode *node, int index, int parent);
    static void ReadMeshData(Model *gameObject, const aiScene *scene,
                             aiNode *node, int bone);
    static unsigned int GetBoneIndex(Model *gameObject,
                                     const std::string &name);
};

} // namespace Engine
#endif
