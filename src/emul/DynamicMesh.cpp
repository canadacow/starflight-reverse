#include "DynamicMesh.hpp"
#include "MapHelper.hpp"

namespace Diligent
{

namespace SF_GLTF
{

DynamicMesh::DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<SF_GLTF::Model>& model) :
    m_Model(model), m_pDevice(pDevice), m_pContext(pContext)
{
    SF_GLTF::Node dynamicMeshNode{0};
    dynamicMeshNode.Name = "DynamicMeshNode";
    dynamicMeshNode.pMesh = nullptr; // No mesh data for now
    dynamicMeshNode.Parent = nullptr; // Root node

    Nodes.emplace_back(dynamicMeshNode);

    // Create a scene and add the node to the scene's root nodes
    SF_GLTF::Scene scene;
    scene.Name = "DynamicMeshScene";
    for (auto& node : Nodes)
    {
        scene.RootNodes.push_back(&node);
        scene.LinearNodes.push_back(&node);
    }

    // Add the scene to the model
    Scenes.push_back(scene);
    DefaultSceneId = static_cast<int>(Scenes.size()) - 1;
}

DynamicMesh::~DynamicMesh()
{
}

bool DynamicMesh::CompatibleWithTransforms(const SF_GLTF::ModelTransforms& Transforms) const
{
    // Call the base class implementation
    bool baseCompatible = SF_GLTF::Model::CompatibleWithTransforms(Transforms);

    // Additional checks specific to DynamicMesh can be added here
    // For now, we assume DynamicMesh is always compatible if the base class is
    return baseCompatible;
}

void DynamicMesh::ComputeTransforms(Uint32           SceneIndex,
                                    SF_GLTF::ModelTransforms& Transforms,
                                    const float4x4& RootTransform,
                                    Int32            AnimationIndex,
                                    float            Time) const
{
    // Call the base class implementation
    SF_GLTF::Model::ComputeTransforms(SceneIndex, Transforms, RootTransform, AnimationIndex, Time);

    // Additional transformations specific to DynamicMesh can be added here
    // For now, we assume no additional transformations are needed
}

BoundBox DynamicMesh::ComputeBoundingBox(Uint32 SceneIndex, const SF_GLTF::ModelTransforms& Transforms, const SF_GLTF::ModelTransforms* DynamicTransforms) const
{
    BoundBox ModelAABB;

    if (m_Model->CompatibleWithTransforms(Transforms))
    {
        VERIFY_EXPR(SceneIndex < m_Model->Scenes.size());
        const auto& scene = m_Model->Scenes[SceneIndex];

        ModelAABB.Min = float3{+FLT_MAX, +FLT_MAX, +FLT_MAX};
        ModelAABB.Max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (const auto* pN : scene.LinearNodes)
        {
            VERIFY_EXPR(pN != nullptr);
            if (pN->pMesh != nullptr && pN->pMesh->IsValidBB())
            {
                const auto& GlobalMatrix = Transforms.NodeGlobalMatrices[pN->Index];
                const auto  NodeAABB     = pN->pMesh->BB.Transform(GlobalMatrix);

                ModelAABB.Min = std::min(ModelAABB.Min, NodeAABB.Min);
                ModelAABB.Max = std::max(ModelAABB.Max, NodeAABB.Max);
            }
        }
    }
    else
    {
        UNEXPECTED("Incompatible transforms. Please use the ComputeTransforms() method first.");
    }

    // Measure the nodes on this object directly
    if (CompatibleWithTransforms(*DynamicTransforms))
    {
        VERIFY_EXPR(SceneIndex < Scenes.size());
        const auto& scene = Scenes[SceneIndex];

        for (const auto* pN : scene.LinearNodes)
        {
            VERIFY_EXPR(pN != nullptr);
            if (pN->pMesh != nullptr && pN->pMesh->IsValidBB())
            {
                const auto& GlobalMatrix = DynamicTransforms->NodeGlobalMatrices[pN->Index];
                const auto  NodeAABB     = pN->pMesh->BB.Transform(GlobalMatrix);

                ModelAABB.Min = std::min(ModelAABB.Min, NodeAABB.Min);
                ModelAABB.Max = std::max(ModelAABB.Max, NodeAABB.Max);
            }
        }
    }
    else
    {
        UNEXPECTED("Incompatible transforms. Please use the ComputeTransforms() method first.");
    }

    return ModelAABB;

}

void DynamicMesh::GeneratePlanes(float width, float height, float tileHeight)
{
    const int numTiles = 8;
    const int numVerticesPerRow = numTiles + 1;
    const int numVertices = numVerticesPerRow * numVerticesPerRow;

    m_Vertices.resize(numVertices); // 4 coordinates per vertex (x, y, z, padding)
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

            auto& vert = m_Vertices[vertexIndex++];
            vert.posX = x;
            vert.posY = y;
            vert.posZ = z;

            // Compute normals
            vert.normalX = 0.0f;
            vert.normalY = 1.0f;
            vert.normalZ = 0.0f;

            // Compute UV coordinates
            vert.texU0 = col / static_cast<float>(numTiles);
            vert.texV0 = row / static_cast<float>(numTiles);

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
    
    Uint32 materialId = 0;
    #if 0
    for (size_t i = 0; i < m_Model->Materials.size(); ++i)
    {
        if (m_Model->Materials[i].Name == "TestPlane")
        {
            materialId = static_cast<Uint32>(i);
            break;
        }
    }
    #endif

    m_Mesh = std::make_shared<SF_GLTF::Mesh>();
    m_Mesh->Primitives.emplace_back(0, m_Indices.size(), m_Vertices.size() / 4, materialId, float3{}, float3{});

    float offsetX = (numBigTiles * width) / 2.0f;
    float offsetY = (numBigTiles * height) / 2.0f;

    auto& node = Nodes[0];
    node.pMesh = m_Mesh.get();

    for (int i = 0; i < numBigTiles; ++i)
    {
        for (int j = 0; j < numBigTiles; ++j)
        {
            NodeInstance ni;
#if 1
            
            ni.NodeMatrix = float4x4::Translation(i * width - offsetX, 0.0f, j * height - offsetY);
            ni.ScaleX = 1.0f / (float)numBigTiles;
            ni.ScaleY = 1.0f / (float)numBigTiles;
            ni.OffsetX = i / (float)numBigTiles;
            ni.OffsetY = j / (float)numBigTiles;
#else
            ni.NodeMatrix = float4x4::Identity();
            ni.ScaleX = 1.0f;
            ni.ScaleY = 1.0f;
            ni.OffsetX = 0.0f;
            ni.OffsetY = 0.0f;
#endif

            node.Instances.push_back(ni);
        }
    }

    CreateBuffers();
    m_GPUDataInitialized = false;

    InitializeVertexAndIndexData();
}

void DynamicMesh::CreateBuffers()
{
    // Initialize vertex buffer
    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Dynamic vertex buffer";
    VertBuffDesc.Usage = USAGE_DYNAMIC;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc.Size = sizeof(VertexBuff) * m_Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc, nullptr, &m_VertexBuffer);

    // Initialize second vertex buffer
    BufferDesc VertBuffDesc2;
    VertBuffDesc2.Name = "Dynamic vertex buffer 2";
    VertBuffDesc2.Usage = USAGE_DYNAMIC;
    VertBuffDesc2.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc2.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc2.Size = sizeof(VertexBuff2) * m_Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc2, nullptr, &m_VertexBuffer2);

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
    //if (m_GPUDataInitialized)
    //    return;

    // Update vertex buffer
    MapHelper<float> Vertices(m_pContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Vertices, m_Vertices.data(), sizeof(VertexBuff) * m_Vertices.size());

    // Update second vertex buffer
    MapHelper<float> Vertices2(m_pContext, m_VertexBuffer2, MAP_WRITE, MAP_FLAG_DISCARD);
    memset(Vertices2, 0, sizeof(VertexBuff2) * m_Vertices.size());

    // Update index buffer
    MapHelper<Uint32> Indices(m_pContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Indices, m_Indices.data(), sizeof(Uint32) * m_Indices.size());

    m_GPUDataInitialized = true;
}

void DynamicMesh::InitializeVertexAndIndexData()
{
    // Initialize VertexData
    VertexData.Strides = { 0x20, 0x20 }; // Assuming 3 floats per vertex (x, y, z)
    VertexData.Buffers = { m_VertexBuffer, m_VertexBuffer2 };
    VertexData.pAllocation = nullptr; // Assuming no suballocation
    VertexData.PoolId = 0;
    VertexData.EnabledAttributeFlags = 0x7; // Position, Normal, Tangent, Color

    // Initialize IndexData
    IndexData.pBuffer = m_IndexBuffer;
    IndexData.pAllocation = nullptr; // Assuming no suballocation
    IndexData.AllocatorId = 0;
    IndexData.IndexSize = sizeof(Uint32);

    NumVertexAttributes = m_Model->GetNumVertexAttributes();
    VertexAttributes = m_Model->VertexAttributes;

    NumTextureAttributes = m_Model->GetNumTextureAttributes();
    TextureAttributes = m_Model->TextureAttributes;
}

} // namespace SF_GLTF

} // namespace Diligent