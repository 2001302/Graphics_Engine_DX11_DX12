#include "geometry_generator.h"

namespace core {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshData GeometryGenerator::MakeSquare(const float scale,
                                       const Vector2 texScale) {
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // ?띿뒪異?醫뚰몴

    // ?욌㈃
    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    MeshData meshData;

    for (size_t i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texcoord = texcoords[i] * texScale;
        v.tangent = Vector3(1.0f, 0.0f, 0.0f);

        // v.color = colors[i];

        meshData.vertices.push_back(v);
    }
    meshData.indices = {
        0, 1, 2, 0, 2, 3, // ?욌㈃
    };

    return meshData;
}

MeshData GeometryGenerator::MakeSquareGrid(const int numSlices,
                                           const int numStacks,
                                           const float scale,
                                           const Vector2 texScale) {
    MeshData meshData;

    float dx = 2.0f / numSlices;
    float dy = 2.0f / numStacks;

    float y = 1.0f;
    for (int j = 0; j < numStacks + 1; j++) {
        float x = -1.0f;
        for (int i = 0; i < numSlices + 1; i++) {
            Vertex v;
            v.position = Vector3(x, y, 0.0f) * scale;
            v.normal = Vector3(0.0f, 0.0f, -1.0f);
            v.texcoord = Vector2(x + 1.0f, y + 1.0f) * 0.5f * texScale;
            v.tangent = Vector3(1.0f, 0.0f, 0.0f);

            meshData.vertices.push_back(v);

            x += dx;
        }
        y -= dy;
    }

    for (int j = 0; j < numStacks; j++) {
        for (int i = 0; i < numSlices; i++) {
            meshData.indices.push_back((numSlices + 1) * j + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i + 1);
        }
    }

    return meshData;
}

MeshData GeometryGenerator::MakeBox(const float scale) {

    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // ?띿뒪異?醫뚰몴

    // ?쀫㈃
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ?꾨옯硫?
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ?욌㈃
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ?룸㈃
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ?쇱そ
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // ?ㅻⅨ履?
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    MeshData meshData;
    for (size_t i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texcoord = texcoords[i];
        meshData.vertices.push_back(v);
    }

    meshData.indices = {
        0,  1,  2,  0,  2,  3,  // ?쀫㈃
        4,  5,  6,  4,  6,  7,  // ?꾨옯硫?
        8,  9,  10, 8,  10, 11, // ?욌㈃
        12, 13, 14, 12, 14, 15, // ?룸㈃
        16, 17, 18, 16, 18, 19, // ?쇱そ
        20, 21, 22, 20, 22, 23  // ?ㅻⅨ履?
    };

    return meshData;
}

MeshData GeometryGenerator::MakeCylinder(const float bottomRadius,
                                         const float topRadius, float height,
                                         int numSlices) {

    // Texture 醫뚰몴怨꾨븣臾몄뿉 (numSlices + 1) x 2 媛쒖쓽 踰꾪뀓???ъ슜

    const float dTheta = -XM_2PI / float(numSlices);

    MeshData meshData;

    vector<Vertex> &vertices = meshData.vertices;

    // ?녿㈃??諛붾떏 踰꾪뀓?ㅻ뱾 (?몃뜳??0 ?댁긽 numSlices 誘몃쭔)
    for (int i = 0; i <= numSlices; i++) {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
                               Matrix::CreateRotationY(dTheta * float(i)));

        // std::cout << v.position.x << " " << v.position.z << std::endl;

        v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
        v.normal.Normalize();
        v.texcoord = Vector2(float(i) / numSlices, 1.0f);

        vertices.push_back(v);
    }

    // ?녿㈃??留???踰꾪뀓?ㅻ뱾 (?몃뜳??numSlices ?댁긽 2 * numSlices 誘몃쭔)
    for (int i = 0; i <= numSlices; i++) {
        Vertex v;
        v.position =
            Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f),
                               Matrix::CreateRotationY(dTheta * float(i)));
        v.normal = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
        v.normal.Normalize();
        v.texcoord = Vector2(float(i) / numSlices, 0.0f);

        vertices.push_back(v);
    }

    vector<uint32_t> &indices = meshData.indices;

    for (int i = 0; i < numSlices; i++) {
        indices.push_back(i);
        indices.push_back(i + numSlices + 1);
        indices.push_back(i + 1 + numSlices + 1);

        indices.push_back(i);
        indices.push_back(i + 1 + numSlices + 1);
        indices.push_back(i + 1);
    }

    return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices,
                                       const int numStacks,
                                       const Vector2 texScale) {

    // 李멸퀬: OpenGL Sphere
    // http://www.songho.ca/opengl/gl_sphere.html
    // Texture 醫뚰몴怨꾨븣臾몄뿉 (numSlices + 1) 媛쒖쓽 踰꾪뀓???ъ슜 (留덉?留됱뿉 ?レ븘二쇰뒗
    // 踰꾪뀓?ㅺ? 以묐났) Stack? y ?꾩そ 諛⑺뼢?쇰줈 ?볦븘媛??諛⑹떇

    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    MeshData meshData;

    vector<Vertex> &vertices = meshData.vertices;

    for (int j = 0; j <= numStacks; j++) {

        // ?ㅽ깮???볦씪 ?섎줉 ?쒖옉?먯쓣 x-y ?됰㈃?먯꽌 ?뚯쟾 ?쒖폒???꾨줈 ?щ━??援ъ“
        Vector3 stackStartPoint = Vector3::Transform(
            Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

        for (int i = 0; i <= numSlices; i++) {
            Vertex v;

            // ?쒖옉?먯쓣 x-z ?됰㈃?먯꽌 ?뚯쟾?쒗궎硫댁꽌 ?먯쓣 留뚮뱶??援ъ“
            v.position = Vector3::Transform(
                stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

            v.normal = v.position; // ?먯젏??援ъ쓽 以묒떖
            v.normal.Normalize();
            v.texcoord =
                Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks) *
                texScale;

            // texcoord媛 ?꾨줈 媛덉닔濡?利앷?
            Vector3 biTangent = Vector3(0.0f, 1.0f, 0.0f);

            Vector3 normalOrth = v.normal - biTangent.Dot(v.normal) * v.normal;
            normalOrth.Normalize();

            v.tangent = biTangent.Cross(normalOrth);
            v.tangent.Normalize();

            /*    Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(dTheta *
               float(i)));*/
            // v.biTangentModel = Vector3(0.0f, 1.0f, 0.0f);

            vertices.push_back(v);
        }
    }

    // cout << vertices.size() << endl;

    vector<uint32_t> &indices = meshData.indices;

    for (int j = 0; j < numStacks; j++) {

        const int offset = (numSlices + 1) * j;

        for (int i = 0; i < numSlices; i++) {

            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    // cout << indices.size() << endl;
    // for (int i = 0; i < indices.size(); i++) {
    //     cout << indices[i] << " ";
    // }
    // cout << endl;

    return meshData;
}

MeshData GeometryGenerator::MakeIcosahedron() {

    // Luna DX12 援먯옱 李멸퀬
    // ??0硫댁껜
    // https://mathworld.wolfram.com/Isohedron.html

    const float X = 0.525731f;
    const float Z = 0.850651f;

    MeshData newMesh;

    vector<Vector3> pos = {
        Vector3(-X, 0.0f, Z), Vector3(X, 0.0f, Z),   Vector3(-X, 0.0f, -Z),
        Vector3(X, 0.0f, -Z), Vector3(0.0f, Z, X),   Vector3(0.0f, Z, -X),
        Vector3(0.0f, -Z, X), Vector3(0.0f, -Z, -X), Vector3(Z, X, 0.0f),
        Vector3(-Z, X, 0.0f), Vector3(Z, -X, 0.0f),  Vector3(-Z, -X, 0.0f)};

    for (size_t i = 0; i < pos.size(); i++) {
        Vertex v;
        v.position = pos[i];
        v.normal = v.position;
        v.normal.Normalize();

        newMesh.vertices.push_back(v);
    }

    newMesh.indices = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                       1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                       3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                       10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

    return newMesh;
}

MeshData GeometryGenerator::MakeTetrahedron() {

    // Regular Tetrahedron
    // https://mathworld.wolfram.com/RegularTetrahedron.html

    const float a = 1.0f;
    const float x = sqrt(3.0f) / 3.0f * a;
    const float d = sqrt(3.0f) / 6.0f * a; // = x / 2
    const float h = sqrt(6.0f) / 3.0f * a;

    vector<Vector3> points = {{0.0f, x, 0.0f},
                              {-0.5f * a, -d, 0.0f},
                              {+0.5f * a, -d, 0.0f},
                              {0.0f, 0.0f, h}};

    Vector3 center = Vector3(0.0f);

    for (int i = 0; i < 4; i++) {
        center += points[i];
    }
    center /= 4.0f;

    for (int i = 0; i < 4; i++) {
        points[i] -= center;
    }

    MeshData meshData;

    for (int i = 0; i < points.size(); i++) {

        Vertex v;
        v.position = points[i];
        v.normal = v.position; // 以묒떖???먯젏
        v.normal.Normalize();

        meshData.vertices.push_back(v);
    }

    meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};

    return meshData;
}
MeshData GeometryGenerator::SubdivideToSphere(const float radius,
                                              MeshData meshData) {

    // ?먯젏??以묒떖?대씪怨?媛??
    for (auto &v : meshData.vertices) {
        v.position = v.normal * radius;
    }

    // 援ъ쓽 ?쒕㈃?쇰줈 ??린怨??몃?怨?texture 醫뚰몴 怨꾩궛
    auto ProjectVertex = [&](Vertex &v) {
        v.normal = v.position;
        v.normal.Normalize();
        v.position = v.normal * radius;

        // 二쇱쓽: ?띿뒪異곌? ?댁쓬留ㅼ뿉??源⑥쭛?덈떎.
        // atan vs atan2
        // https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
        // const float theta = atan2f(v.position.z, v.position.x);
        // const float phi = acosf(v.position.y / radius);
        // v.texcoord.x = theta / XM_2PI;
        // v.texcoord.y = phi / XM_PI;
    };

    auto UpdateFaceNormal = [](Vertex &v0, Vertex &v1, Vertex &v2) {
        auto faceNormal =
            (v1.position - v0.position).Cross(v2.position - v0.position);
        faceNormal.Normalize();
        v0.normal = faceNormal;
        v1.normal = faceNormal;
        v2.normal = faceNormal;
    };

    // 踰꾪뀓?ㅺ? 以묐났?섎뒗 援ъ“濡?援ы쁽
    MeshData newMesh;
    uint32_t count = 0;
    for (size_t i = 0; i < meshData.indices.size(); i += 3) {
        size_t i0 = meshData.indices[i];
        size_t i1 = meshData.indices[i + 1];
        size_t i2 = meshData.indices[i + 2];

        Vertex v0 = meshData.vertices[i0];
        Vertex v1 = meshData.vertices[i1];
        Vertex v2 = meshData.vertices[i2];

        Vertex v3;
        v3.position = (v0.position + v2.position) * 0.5f;
        v3.texcoord = (v0.texcoord + v2.texcoord) * 0.5f;
        ProjectVertex(v3);

        Vertex v4;
        v4.position = (v0.position + v1.position) * 0.5f;
        v4.texcoord = (v0.texcoord + v1.texcoord) * 0.5f;
        ProjectVertex(v4);

        Vertex v5;
        v5.position = (v1.position + v2.position) * 0.5f;
        v5.texcoord = (v1.texcoord + v2.texcoord) * 0.5f;
        ProjectVertex(v5);

        // UpdateFaceNormal(v4, v1, v5);
        // UpdateFaceNormal(v0, v4, v3);
        // UpdateFaceNormal(v3, v4, v5);
        // UpdateFaceNormal(v3, v5, v2);

        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v1);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v0);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v3);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v5);
        newMesh.vertices.push_back(v2);

        for (uint32_t j = 0; j < 12; j++) {
            newMesh.indices.push_back(j + count);
        }
        count += 12;
    }

    return newMesh;
}
vector<MeshData> GeometryGenerator::ReadFromFile(std::string basePath,
                                                 std::string filename,
                                                 bool revertNormals) {

    using namespace DirectX;

    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);
    vector<MeshData> &meshes = modelLoader.m_meshes;

    // Normalize vertices
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float dl = XMMax(XMMax(dx, dy), dz);
    float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
          cz = (vmax.z + vmin.z) * 0.5f;

    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            v.position.x = (v.position.x - cx) / dl;
            v.position.y = (v.position.y - cy) / dl;
            v.position.z = (v.position.z - cz) / dl;
        }
    }

    return meshes;
}

MeshData GeometryGenerator::MakeWireBox(const Vector3 center,
                                        const Vector3 extents) {

    // ?곸옄瑜???댁뼱 ?꾨젅?꾩쑝濡?洹몃━???⑸룄

    vector<Vector3> positions;

    // ?욌㈃
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extents);

    // ?룸㈃
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extents);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extents);

    MeshData meshData;
    for (size_t i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.normal = positions[i] - center;
        v.normal.Normalize();
        v.texcoord = Vector2(0.0f); // 誘몄궗??
        meshData.vertices.push_back(v);
    }

    // Line list
    meshData.indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // ?욌㈃
        4, 5, 5, 6, 6, 7, 7, 4, // ?룸㈃
        0, 4, 1, 5, 2, 6, 3, 7  // ?녿㈃
    };

    return meshData;
}

MeshData GeometryGenerator::MakeWireSphere(const Vector3 center,
                                           const float radius) {
    MeshData meshData;
    vector<Vertex> &vertices = meshData.vertices;
    vector<uint32_t> &indices = meshData.indices;

    const int numPoints = 30;
    const float dTheta = XM_2PI / float(numPoints);

    // XY plane
    {
        int offset = int(vertices.size());
        Vector3 start(1.0f, 0.0f, 0.0f);
        for (int i = 0; i < numPoints; i++) {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationZ(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0) {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // YZ
    {
        int offset = int(vertices.size());
        Vector3 start(0.0f, 1.0f, 0.0f);
        for (int i = 0; i < numPoints; i++) {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationX(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0) {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // XZ
    {
        int offset = int(vertices.size());
        Vector3 start(1.0f, 0.0f, 0.0f);
        for (int i = 0; i < numPoints; i++) {
            Vertex v;
            v.position =
                center + Vector3::Transform(start, Matrix::CreateRotationY(
                                                       dTheta * float(i))) *
                             radius;
            vertices.push_back(v);
            indices.push_back(i + offset);
            if (i != 0) {
                indices.push_back(i + offset);
            }
        }
        indices.push_back(offset);
    }

    // for (auto &v : vertices) {
    //     cout << v.position.x << " " << v.position.y << " " << v.position.z
    //          << endl;
    // }

    // for (int i = 0; i < indices.size(); i++) {
    //     cout << indices[i] << " ";
    // }
    // cout << endl;

    return meshData;
}

auto GeometryGenerator::ReadAnimationFromFile(string basePath, string filename,
                                              bool revertNormals)
    -> tuple<vector<MeshData>, AnimationData> {

    ModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);

    GeometryGenerator::Normalize(Vector3(0.0f), 1.0f, modelLoader.m_meshes,
                                 modelLoader.m_aniData);

    return {modelLoader.m_meshes, modelLoader.m_aniData};
}

void GeometryGenerator::Normalize(const Vector3 center,
                                  const float longestLength,
                                  vector<MeshData> &meshes,
                                  AnimationData &aniData) {

    // 紐⑤뜽??以묒떖???먯젏?쇰줈 ??린怨??ш린瑜?[-1,1]^3?쇰줈 ?ㅼ???

    using namespace DirectX;

    // Normalize vertices
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float scale = longestLength / XMMax(XMMax(dx, dy), dz);
    Vector3 translation = -(vmin + vmax) * 0.5f + center;

    for (auto &mesh : meshes) {
        for (auto &v : mesh.vertices) {
            v.position = (v.position + translation) * scale;
        }

        for (auto &v : mesh.skinned_vertices) {
            v.position = (v.position + translation) * scale;
        }
    }

    // ?좊땲硫붿씠???곗씠??蹂댁젙???ъ슜
    aniData.defaultTransform =
        Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
}

} // namespace engine
