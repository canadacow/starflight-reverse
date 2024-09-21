#include "DynamicMesh.hpp"
#include "MapHelper.hpp"

namespace Diligent
{

DynamicMesh::DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const GLTF::Model& model) :
    m_Model(model)
{
    // Initialize vertex and index buffers
    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Dynamic vertex buffer";
    VertBuffDesc.Usage = USAGE_DYNAMIC;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc.Size = sizeof(float) * m_Vertices.size();
    pDevice->CreateBuffer(VertBuffDesc, nullptr, &m_VertexBuffer);

    BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Dynamic index buffer";
    IndBuffDesc.Usage = USAGE_DYNAMIC;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    IndBuffDesc.Size = sizeof(Uint32) * m_Indices.size();
    pDevice->CreateBuffer(IndBuffDesc, nullptr, &m_IndexBuffer);
}

DynamicMesh::~DynamicMesh()
{
}

void DynamicMesh::PrepareGPUResources(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    if (m_GPUDataInitialized)
        return;

    // Update vertex buffer
    MapHelper<float> Vertices(pContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Vertices, m_Vertices.data(), sizeof(float) * m_Vertices.size());

    // Update index buffer
    MapHelper<Uint32> Indices(pContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Indices, m_Indices.data(), sizeof(Uint32) * m_Indices.size());

    m_GPUDataInitialized = true;
}

void DynamicMesh::Draw(IDeviceContext* pContext)
{
    // Bind vertex and index buffers
    IBuffer* pBuffs[] = { m_VertexBuffer };
    Uint64 offsets[] = { 0 }; // Change Uint32 to Uint64
    pContext->SetVertexBuffers(0, 1, pBuffs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    pContext->SetIndexBuffer(m_IndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Draw call
    DrawIndexedAttribs DrawAttrs;
    DrawAttrs.IndexType = VT_UINT32;
    DrawAttrs.NumIndices = static_cast<Uint32>(m_Indices.size());
    pContext->DrawIndexed(DrawAttrs);
}

void DynamicMesh::UpdateVertexData(const std::vector<float>& vertices)
{
    m_Vertices = vertices;
    m_GPUDataInitialized = false;
}

void DynamicMesh::UpdateIndexData(const std::vector<Uint32>& indices)
{
    m_Indices = indices;
    m_GPUDataInitialized = false;
}

} // namespace Diligent