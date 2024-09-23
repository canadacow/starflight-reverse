#pragma once

#include "../pbr/SF_GLTFLoader.hpp"
#include "BasicTypes.h"

namespace Diligent
{

class DynamicMesh : public SF_GLTF::Model
{
public:
    DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<SF_GLTF::Model>& model);
    ~DynamicMesh();

    void PrepareResources();

    void CreateBuffers();

    void UpdateVertexData(const std::vector<float>& vertices);
    void UpdateIndexData(const std::vector<Uint32>& indices);
    void GeneratePlane(float width, float height, float tileHeight);

    const std::vector<SF_GLTF::Material>& GetMaterials() const override
    {
        return m_Model->GetMaterials();
    }

    std::vector<SF_GLTF::Material>& GetMaterials() override
    {
        return m_Model->GetMaterials();
    }

private:

    RefCntAutoPtr<IBuffer> m_VertexBuffer;
    RefCntAutoPtr<IBuffer> m_IndexBuffer;
    std::shared_ptr<SF_GLTF::Model> m_Model;
    IRenderDevice* m_pDevice;
    IDeviceContext* m_pContext; 

    std::vector<float> m_Vertices;
    std::vector<Uint32> m_Indices;

    bool m_GPUDataInitialized = false;
};

} // namespace Diligent