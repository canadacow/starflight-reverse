#pragma once

#include "../pbr/SF_GLTFLoader.hpp"
#include "BasicTypes.h"

namespace Diligent
{

class DynamicMesh : public GLTF::Model
{
public:
    DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<GLTF::Model>& model);
    ~DynamicMesh();

    void PrepareResources();

    void CreateBuffers();

    void UpdateVertexData(const std::vector<float>& vertices);
    void UpdateIndexData(const std::vector<Uint32>& indices);
    void GeneratePlane(float width, float height, float tileHeight);

private:

    RefCntAutoPtr<IBuffer> m_VertexBuffer;
    RefCntAutoPtr<IBuffer> m_IndexBuffer;
    std::shared_ptr<GLTF::Model> m_Model;
    IRenderDevice* m_pDevice;
    IDeviceContext* m_pContext; 

    std::vector<float> m_Vertices;
    std::vector<Uint32> m_Indices;

    bool m_GPUDataInitialized = false;
};

} // namespace Diligent