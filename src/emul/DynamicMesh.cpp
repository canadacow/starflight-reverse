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
    dynamicMeshNode.isHeightmap = true;
    dynamicMeshNode.isTerrain = true;

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

    // Calculate the bounding box for the mesh
    float3 minBB = float3{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    float3 maxBB = float3{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

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

            minBB.x = std::min(minBB.x, vert.posX);
            minBB.y = std::min(minBB.y, vert.posY);
            minBB.z = std::min(minBB.z, vert.posZ);

            maxBB.x = std::max(maxBB.x, vert.posX);
            maxBB.y = std::max(maxBB.y, vert.posY);
            maxBB.z = std::max(maxBB.z, vert.posZ);            

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

    float3 baseMinBB = minBB;
    float3 baseMaxBB = maxBB;

    for (int i = 0; i < numBigTiles; ++i)
    {
        for (int j = 0; j < numBigTiles; ++j)
        {
            NodeInstance ni;
            ni.NodeMatrix = float4x4::Translation(i * width - offsetX, 0.0f, j * height - offsetY);
            ni.ScaleX = 1.0f / (float)numBigTiles;
            ni.ScaleY = 1.0f / (float)numBigTiles;
            ni.OffsetX = i / (float)numBigTiles;
            ni.OffsetY = j / (float)numBigTiles;
            node.Instances.push_back(ni);

            float3 minTranslatedPos = baseMinBB * ni.NodeMatrix;
            float3 maxTranslatedPos = baseMaxBB * ni.NodeMatrix;

            minBB = float3{
                std::min(minBB.x, minTranslatedPos.x),
                std::min(minBB.y, minTranslatedPos.y),
                std::min(minBB.z, minTranslatedPos.z)
            };

            maxBB = float3{
                std::max(maxBB.x, maxTranslatedPos.x),
                std::max(maxBB.y, maxTranslatedPos.y),
                std::max(maxBB.z, maxTranslatedPos.z)
            };

        }
    }

    float yValues[] = {-2.0f, 10.0f};

    for (float y : yValues)
    {
        float3 minTranslatedPos = baseMinBB * float4x4::Translation(0.0f, y, 0.0f);
        float3 maxTranslatedPos = baseMaxBB * float4x4::Translation(0.0f, y, 0.0f);

        minBB = float3{
            std::min(minBB.x, minTranslatedPos.x),
            std::min(minBB.y, minTranslatedPos.y),
            std::min(minBB.z, minTranslatedPos.z)
        };

        maxBB = float3{
            std::max(maxBB.x, maxTranslatedPos.x),
            std::max(maxBB.y, maxTranslatedPos.y),
            std::max(maxBB.z, maxTranslatedPos.z)
        };
    }

    m_Mesh->BB.Min = minBB;
    m_Mesh->BB.Max = maxBB;

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

    // Initialize third vertex buffer
    BufferDesc VertBuffDesc3;
    VertBuffDesc3.Name = "Dynamic vertex buffer 3";
    VertBuffDesc3.Usage = USAGE_DYNAMIC;
    VertBuffDesc3.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc3.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc3.Size = sizeof(VertexBuff3) * m_Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc3, nullptr, &m_VertexBuffer3);

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

    // Update third vertex buffer
    MapHelper<float> Vertices3(m_pContext, m_VertexBuffer3, MAP_WRITE, MAP_FLAG_DISCARD);
    memset(Vertices3, 0, sizeof(VertexBuff3) * m_Vertices.size());

    // Update index buffer
    MapHelper<Uint32> Indices(m_pContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    memcpy(Indices, m_Indices.data(), sizeof(Uint32) * m_Indices.size());

    m_GPUDataInitialized = true;
}

void DynamicMesh::InitializeVertexAndIndexData()
{
    // Initialize VertexData
    VertexData.Strides = { 0x20, 0x20, 0x20 }; // Assuming 3 floats per vertex (x, y, z)
    VertexData.Buffers = { m_VertexBuffer, m_VertexBuffer2, m_VertexBuffer3 };
    VertexData.pAllocation = nullptr; // Assuming no suballocation
    VertexData.PoolId = 0;
    VertexData.EnabledAttributeFlags = 0xf; // Position, Normal, Texcoord0, Texcoord1

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

void DynamicMesh::SetTerrainItems(const TerrainItems& terrainItems)
{
    ClearTerrainItems();

    for (const auto& item : terrainItems)
    {
        auto it = std::find_if(m_Model->Nodes.begin(), m_Model->Nodes.end(), [&item](const auto& node) {
            return std::equal(node.Name.begin(), node.Name.end(), item.name.begin(), item.name.end(),
                            [](char a, char b) { return tolower(a) == tolower(b); });
        });

        if (it != m_Model->Nodes.end())
        {
            Node ourNode{static_cast<int>(Nodes.size())};

            ourNode.Name = it->Name;
            ourNode.pMesh = it->pMesh;
            ourNode.Translation = item.position;
            ourNode.Rotation = item.rotation;

            Nodes.emplace_back(ourNode);
            Scenes[0].LinearNodes.push_back(&Nodes.back());
            Scenes[0].RootNodes.push_back(&Nodes.back());
        }
    }
}

void DynamicMesh::ClearTerrainItems()
{
    Scenes[0].LinearNodes.erase(Scenes[0].LinearNodes.begin() + 1, Scenes[0].LinearNodes.end());
    Scenes[0].RootNodes.erase(Scenes[0].RootNodes.begin() + 1, Scenes[0].RootNodes.end());
    Nodes.erase(Nodes.begin() + 1, Nodes.end());
}

void DynamicMesh::SetVertexBuffersForNode(IDeviceContext* pCtx, const Node* node) const
{
    std::array<IBuffer*, 8> pVBs;

    if (node == &Nodes[0] && targetNode != node)
    {
        // Terrain mesh
        const auto NumVBs = static_cast<Uint32>(GetVertexBufferCount());
        VERIFY_EXPR(NumVBs <= pVBs.size());
        for (Uint32 i = 0; i < NumVBs; ++i)
            pVBs[i] = GetVertexBuffer(i);
        pCtx->SetVertexBuffers(0, NumVBs, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        if (auto* pIndexBuffer = GetIndexBuffer())
        {
            pCtx->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }
    else if(targetNode != node)
    {
        // Other meshes
        const auto NumVBs = static_cast<Uint32>(m_Model->GetVertexBufferCount());
        VERIFY_EXPR(NumVBs <= pVBs.size());
        for (Uint32 i = 0; i < NumVBs; ++i)
            pVBs[i] = m_Model->GetVertexBuffer(i);
        pCtx->SetVertexBuffers(0, NumVBs, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        if (auto* pIndexBuffer = m_Model->GetIndexBuffer())
        {
            pCtx->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }        
    }

    targetNode = node;
}

Uint32 DynamicMesh::GetFirstIndexLocationForNode(const Node* node) const
{
    if(node == &Nodes[0])
    {
        return GetFirstIndexLocation();
    }
    else
    {
        return m_Model->GetFirstIndexLocation();
    }
}

Uint32 DynamicMesh::GetBaseVertexForNode(const Node* node) const
{
    if(node == &Nodes[0])
    {
        return GetBaseVertex();
    }
    else
    {
        return m_Model->GetBaseVertex();
    }
}

Uint32 DynamicMesh::GetNumVertexAttributesForNode(const Node* node) const
{
    if(node == &Nodes[0])
    {
        return GetNumVertexAttributes();
    }
    else
    {
        return m_Model->GetNumVertexAttributes();
    }
}

const VertexAttributeDesc DynamicMesh::GetVertexAttributeForNode(const Node* node, size_t Idx) const
{
    if(node == &Nodes[0])
    {
        return GetVertexAttribute(Idx);
    }
    else
    {
        return m_Model->GetVertexAttribute(Idx);
    }
}

 bool DynamicMesh::IsVertexAttributeEnabledForNode(const Node* node, Uint32 AttribId) const
 {
    if(node == &Nodes[0])
    {
        return IsVertexAttributeEnabled(AttribId);
    }
    else
    {
        return m_Model->IsVertexAttributeEnabled(AttribId);
    }
 }

} // namespace SF_GLTF

} // namespace Diligent