#pragma once

#include "GLTFLoader.hpp"
#include "BasicTypes.h"

namespace Diligent
{

class DynamicMesh
{
public:
    DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const GLTF::Model& model);
    ~DynamicMesh();

    void PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pContext);
    void Draw(IDeviceContext* pContext);

    void UpdateVertexData(const std::vector<float>& vertices);
    void UpdateIndexData(const std::vector<Uint32>& indices);

private:
    RefCntAutoPtr<IBuffer> m_VertexBuffer;
    RefCntAutoPtr<IBuffer> m_IndexBuffer;
    const GLTF::Model& m_Model;

    std::vector<float> m_Vertices;
    std::vector<Uint32> m_Indices;

    bool m_GPUDataInitialized = false;
};

} // namespace Diligent