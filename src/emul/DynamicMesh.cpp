#include "DynamicMesh.hpp"
#include "MapHelper.hpp"

namespace Diligent
{

DynamicMesh::DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<GLTF::Model>& model) :
    m_Model(model), m_pDevice(pDevice), m_pContext(pContext)
{
    // Buffers will be created after generating the plane
    
    //GLTF::Mesh dynamicMesh;
    //dynamicMesh.Name = "DynamicMesh";
    //Meshes.push_back(dynamicMesh);

    //CopyMaterials(model->Materials, Materials);
}

DynamicMesh::~DynamicMesh()
{
}

void DynamicMesh::GeneratePlane(float width, float height, float tileHeight)
{
    const int numTiles = 8;
    const int numVerticesPerRow = numTiles + 1;
    const int numVertices = numVerticesPerRow * numVerticesPerRow;

    m_Vertices.resize(numVertices * 3); // 3 coordinates per vertex (x, y, z)
    m_Indices.resize(numTiles * numTiles * 6); // 6 indices per tile (2 triangles)

    int vertexIndex = 0;
    int indexIndex = 0;

    for (int row = 0; row <= numTiles; ++row)
    {
        for (int col = 0; col <= numTiles; ++col)
        {
            float x = (col / static_cast<float>(numTiles)) * width;
            float z = (row / static_cast<float>(numTiles)) * height;
            float y = tileHeight;

            m_Vertices[vertexIndex++] = x;
            m_Vertices[vertexIndex++] = y;
            m_Vertices[vertexIndex++] = z;

            if (row < numTiles && col < numTiles)
            {
                int topLeft = row * numVerticesPerRow + col;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + numVerticesPerRow;
                int bottomRight = bottomLeft + 1;

                // First triangle
                m_Indices[indexIndex++] = topLeft;
                m_Indices[indexIndex++] = bottomLeft;
                m_Indices[indexIndex++] = topRight;

                // Second triangle
                m_Indices[indexIndex++] = topRight;
                m_Indices[indexIndex++] = bottomLeft;
                m_Indices[indexIndex++] = bottomRight;
            }
        }
    }

    CreateBuffers();
    m_GPUDataInitialized = false;
}

void DynamicMesh::CreateBuffers()
{
    // Initialize vertex buffer
    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Dynamic vertex buffer";
    VertBuffDesc.Usage = USAGE_DYNAMIC;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc.Size = sizeof(float) * m_Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc, nullptr, &m_VertexBuffer);

    // Initialize index buffer
    BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Dynamic index buffer";
    IndBuffDesc.Usage = USAGE_DYNAMIC;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    IndBuffDesc.Size = sizeof(Uint32) * m_Indices.size();
    m_pDevice->CreateBuffer(IndBuffDesc, nullptr, &m_IndexBuffer);
}

void DynamicMesh::PrepareResources()
{
    if (m_GPUDataInitialized)
        return;

    // Update vertex buffer
    MapHelper<float> Vertices(m_pContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Vertices, m_Vertices.data(), sizeof(float) * m_Vertices.size());

    // Update index buffer
    MapHelper<Uint32> Indices(m_pContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Indices, m_Indices.data(), sizeof(Uint32) * m_Indices.size());

    m_GPUDataInitialized = true;
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