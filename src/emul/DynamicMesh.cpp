#include "DynamicMesh.hpp"
#include "MapHelper.hpp"

#include <Windows.h>

#include <assert.h>

extern float bicubicOffset;

namespace Diligent
{

namespace SF_GLTF
{

static constexpr float TERRAIN_MAX_X = 2304.0f;
static constexpr float TERRAIN_MAX_Y = 960.0f;

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
#if 0
    BoundBox ModelAABB;

    ModelAABB.Min = float3{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    ModelAABB.Max = float3{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

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

                // If this node has instances, transform the AABB for each instance
                if (!pN->Instances.empty())
                {
                    for (const auto& instance : pN->Instances)
                    {
                        const auto InstanceAABB = NodeAABB.Transform(instance.NodeMatrix);
                        ModelAABB.Min = min(ModelAABB.Min, InstanceAABB.Min);
                        ModelAABB.Max = max(ModelAABB.Max, InstanceAABB.Max);
                    }
                    continue; // Skip adding the original node AABB since we've handled all instances
                }

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
#endif
    return m_Mesh->BB;
}

void DynamicMesh::GeneratePlanes(float width, float height, float tileHeight, float2 textureSize)
{
    const int numQuadsPerTile = 8; // Current high-density mesh is 8x8 quads
    const int numVerticesPerRow = numQuadsPerTile + 1;
    const int numVertices = numVerticesPerRow * numVerticesPerRow;
    m_TileSize = float2{ width, height };
    m_TextureSize = textureSize;

    VertexIndexCounts counts = {};

    // Generate high LOD mesh (current implementation)
    generateHighLODMesh(counts, numQuadsPerTile, tileHeight);
    
    // Generate medium LOD mesh (single quad per tile)
    generateMediumLODMesh(counts, tileHeight);
    
    // Generate low LOD mesh (large quads covering 61x61 tile areas)
    generateLowLODMesh(counts, tileHeight);

    generateNonInstancedMesh(counts, tileHeight);

    CreateBuffers();
    m_GPUDataInitialized = false;

    InitializeVertexAndIndexData();
}

void DynamicMesh::generateHighLODMesh(VertexIndexCounts& counts, int numQuadsPerTile, float tileHeight)
{
    const int numVerticesPerRow = numQuadsPerTile + 1;
    const int numVertices = numVerticesPerRow * numVerticesPerRow;
    
    m_HighLODVertices.resize(numVertices);
    m_HighLODIndices.resize(numQuadsPerTile * numQuadsPerTile * 6); // 6 indices per quad (2 triangles)

    int vertexIndex = 0;
    int indexIndex = 0;

    m_HighLODOffsets.vertexCount = vertexIndex;
    m_HighLODOffsets.indexCount = indexIndex;

    for (int row = 0; row <= numQuadsPerTile; ++row)
    {
        for (int col = 0; col <= numQuadsPerTile; ++col)
        {
            float x = (col / static_cast<float>(numQuadsPerTile)) * m_TileSize.x;
            float z = (row / static_cast<float>(numQuadsPerTile)) * m_TileSize.y;
            float y = tileHeight;

            auto& vert = m_HighLODVertices[vertexIndex++];
            vert.posX = x;
            vert.posY = y;
            vert.posZ = z;

            // Compute normals
            vert.normalX = 0.0f;
            vert.normalY = 1.0f;
            vert.normalZ = 0.0f;

            // Compute UV coordinates
            vert.texU0 = col / static_cast<float>(numQuadsPerTile);
            vert.texV0 = row / static_cast<float>(numQuadsPerTile);

            if (row < numQuadsPerTile && col < numQuadsPerTile)
            {
                int topLeft = row * numVerticesPerRow + col;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + numVerticesPerRow;
                int bottomRight = bottomLeft + 1;

                // First triangle
                m_HighLODIndices[indexIndex++] = topLeft;
                m_HighLODIndices[indexIndex++] = bottomLeft;
                m_HighLODIndices[indexIndex++] = topRight;

                // Second triangle
                m_HighLODIndices[indexIndex++] = topRight;
                m_HighLODIndices[indexIndex++] = bottomLeft;
                m_HighLODIndices[indexIndex++] = bottomRight;
            }
        }
    }

    counts.vertexCount = vertexIndex;
    counts.indexCount = indexIndex;
}

void DynamicMesh::generateMediumLODMesh(VertexIndexCounts& counts, float tileHeight)
{
    // Medium LOD - single quad per tile
    const int numVerticesPerQuad = 4;
    const int numIndicesPerQuad = 6;

    m_MediumLODOffsets.vertexCount = counts.vertexCount;
    m_MediumLODOffsets.indexCount = counts.indexCount;
    
    m_MediumLODVertices.resize(numVerticesPerQuad);
    m_MediumLODIndices.resize(numIndicesPerQuad);

    // Create a single quad covering the entire tile
    // Top-left vertex
    m_MediumLODVertices[0].posX = 0.0f;
    m_MediumLODVertices[0].posY = tileHeight;
    m_MediumLODVertices[0].posZ = 0.0f;
    m_MediumLODVertices[0].normalX = 0.0f;
    m_MediumLODVertices[0].normalY = 1.0f;
    m_MediumLODVertices[0].normalZ = 0.0f;
    m_MediumLODVertices[0].texU0 = 0.0f;
    m_MediumLODVertices[0].texV0 = 0.0f;

    // Top-right vertex
    m_MediumLODVertices[1].posX = m_TileSize.x;
    m_MediumLODVertices[1].posY = tileHeight;
    m_MediumLODVertices[1].posZ = 0.0f;
    m_MediumLODVertices[1].normalX = 0.0f;
    m_MediumLODVertices[1].normalY = 1.0f;
    m_MediumLODVertices[1].normalZ = 0.0f;
    m_MediumLODVertices[1].texU0 = 1.0f;
    m_MediumLODVertices[1].texV0 = 0.0f;

    // Bottom-left vertex
    m_MediumLODVertices[2].posX = 0.0f;
    m_MediumLODVertices[2].posY = tileHeight;
    m_MediumLODVertices[2].posZ = m_TileSize.y;
    m_MediumLODVertices[2].normalX = 0.0f;
    m_MediumLODVertices[2].normalY = 1.0f;
    m_MediumLODVertices[2].normalZ = 0.0f;
    m_MediumLODVertices[2].texU0 = 0.0f;
    m_MediumLODVertices[2].texV0 = 1.0f;

    // Bottom-right vertex
    m_MediumLODVertices[3].posX = m_TileSize.x;
    m_MediumLODVertices[3].posY = tileHeight;
    m_MediumLODVertices[3].posZ = m_TileSize.y;
    m_MediumLODVertices[3].normalX = 0.0f;
    m_MediumLODVertices[3].normalY = 1.0f;
    m_MediumLODVertices[3].normalZ = 0.0f;
    m_MediumLODVertices[3].texU0 = 1.0f;
    m_MediumLODVertices[3].texV0 = 1.0f;

    // First triangle
    m_MediumLODIndices[0] = counts.vertexCount + 0; // top-left
    m_MediumLODIndices[1] = counts.vertexCount + 2; // bottom-left
    m_MediumLODIndices[2] = counts.vertexCount + 1; // top-right

    // Second triangle
    m_MediumLODIndices[3] = counts.vertexCount + 1; // top-right
    m_MediumLODIndices[4] = counts.vertexCount + 2; // bottom-left
    m_MediumLODIndices[5] = counts.vertexCount + 3; // bottom-right

    counts.indexCount += 6;
    counts.vertexCount += 4;
}

void DynamicMesh::generateLowLODMesh(VertexIndexCounts& counts, float tileHeight)
{
    // Low LOD - single quad covering a 61x61 tile area
    const int numVerticesPerQuad = 4;
    const int numIndicesPerQuad = 6;

    m_LowLODOffsets.vertexCount = counts.vertexCount;
    m_LowLODOffsets.indexCount = counts.indexCount;
    
    m_LowLODVertices.resize(numVerticesPerQuad);
    m_LowLODIndices.resize(numIndicesPerQuad);

    // Size of a low LOD quad (covers numBigTiles x numBigTiles area)
    float2 lowLODSize = m_TileSize * float2{ (float)numBigTiles.x, (float)numBigTiles.y };

    // Create a single quad covering a 61x61 tile area
    // Top-left vertex
    m_LowLODVertices[0].posX = 0.0f;
    m_LowLODVertices[0].posY = tileHeight;
    m_LowLODVertices[0].posZ = 0.0f;
    m_LowLODVertices[0].normalX = 0.0f;
    m_LowLODVertices[0].normalY = 1.0f;
    m_LowLODVertices[0].normalZ = 0.0f;
    m_LowLODVertices[0].texU0 = 0.0f;
    m_LowLODVertices[0].texV0 = 0.0f;

    // Top-right vertex
    m_LowLODVertices[1].posX = lowLODSize.x;
    m_LowLODVertices[1].posY = tileHeight;
    m_LowLODVertices[1].posZ = 0.0f;
    m_LowLODVertices[1].normalX = 0.0f;
    m_LowLODVertices[1].normalY = 1.0f;
    m_LowLODVertices[1].normalZ = 0.0f;
    m_LowLODVertices[1].texU0 = 1.0f;
    m_LowLODVertices[1].texV0 = 0.0f;

    // Bottom-left vertex
    m_LowLODVertices[2].posX = 0.0f;
    m_LowLODVertices[2].posY = tileHeight;
    m_LowLODVertices[2].posZ = lowLODSize.y;
    m_LowLODVertices[2].normalX = 0.0f;
    m_LowLODVertices[2].normalY = 1.0f;
    m_LowLODVertices[2].normalZ = 0.0f;
    m_LowLODVertices[2].texU0 = 0.0f;
    m_LowLODVertices[2].texV0 = 1.0f;

    // Bottom-right vertex
    m_LowLODVertices[3].posX = lowLODSize.x;
    m_LowLODVertices[3].posY = tileHeight;
    m_LowLODVertices[3].posZ = lowLODSize.y;
    m_LowLODVertices[3].normalX = 0.0f;
    m_LowLODVertices[3].normalY = 1.0f;
    m_LowLODVertices[3].normalZ = 0.0f;
    m_LowLODVertices[3].texU0 = 1.0f;
    m_LowLODVertices[3].texV0 = 1.0f;

    // First triangle
    m_LowLODIndices[0] = counts.vertexCount + 0; // top-left
    m_LowLODIndices[1] = counts.vertexCount + 2; // bottom-left
    m_LowLODIndices[2] = counts.vertexCount + 1; // top-right

    // Second triangle
    m_LowLODIndices[3] = counts.vertexCount + 1; // top-right
    m_LowLODIndices[4] = counts.vertexCount + 2; // bottom-left
    m_LowLODIndices[5] = counts.vertexCount + 3; // bottom-right

    counts.indexCount += 6;
    counts.vertexCount += 4;
}

void DynamicMesh::generateNonInstancedMesh(VertexIndexCounts& counts, float tileHeight)
{
    // Clear existing vertices and indices
    m_NonInstancedVertices.clear();
    m_NonInstancedIndices.clear();

    m_NonInstancedOffsets.vertexCount = counts.vertexCount;
    m_NonInstancedOffsets.indexCount = counts.indexCount;
    
    // Calculate the size of the entire grid of tiles
    float totalWidth = TERRAIN_MAX_X * m_TileSize.x;
    float totalHeight = TERRAIN_MAX_Y * m_TileSize.y;
    
    // Calculate how many low LOD tiles we need to cover one terrain section
    // Each low LOD tile covers numBigTiles x numBigTiles area
    int lowLODTilesX = (TERRAIN_MAX_X + numBigTiles.x - 1) / numBigTiles.x;
    int lowLODTilesY = (TERRAIN_MAX_Y + numBigTiles.y - 1) / numBigTiles.y;
    
    // Create 3x5 repeating pattern of the terrain (3 columns, 5 rows)
    for (int repeatX = -1; repeatX <= 1; ++repeatX) {
        for (int repeatY = -3; repeatY <= 3; ++repeatY) {
            // Calculate the offset for this terrain section
            float xOffset = repeatX * totalWidth;
            float zOffset = repeatY * totalHeight;
            
            // Create low LOD tiles to cover this terrain section
            for (int x = 0; x < lowLODTilesX; ++x) {
                for (int y = 0; y < lowLODTilesY; ++y) {
                    // Calculate the actual tile position
                    int2 tile = int2{x * numBigTiles.x, y * numBigTiles.y};
                    
                    // Calculate position for this low LOD tile
                    float posX = xOffset + (float)tile.x * m_TileSize.x;
                    float posZ = zOffset + (float)tile.y * m_TileSize.y;
                   
                    // Generate vertices for this tile
                    Uint32 baseIndex = static_cast<Uint32>(m_NonInstancedVertices.size()) + m_NonInstancedOffsets.vertexCount;
                    
                    // Add four vertices for this tile (quad)
                    // Bottom-left
                    m_NonInstancedVertices.push_back({
                        posX, tileHeight, posZ,                      // Position
                        0.0f, 1.0f, 0.0f,                            // Normal
                        0.0f, 0.0f                             // UV
                    });
                    
                    // Bottom-right
                    m_NonInstancedVertices.push_back({
                        posX + numBigTiles.x * m_TileSize.x, tileHeight, posZ,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 0.0f, // UV
                    });
                    
                    // Top-right
                    m_NonInstancedVertices.push_back({
                        posX + numBigTiles.x * m_TileSize.x, tileHeight, posZ + numBigTiles.y * m_TileSize.y,
                        0.0f, 1.0f, 0.0f,
                        1.0f, 1.0f // UV
                    });
                    
                    // Top-left
                    m_NonInstancedVertices.push_back({
                        posX, tileHeight, posZ + numBigTiles.y * m_TileSize.y,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 1.0f // UV
                    });
                    
                    // Add indices for two triangles (forming a quad)
                    m_NonInstancedIndices.push_back(baseIndex);
                    m_NonInstancedIndices.push_back(baseIndex + 1);
                    m_NonInstancedIndices.push_back(baseIndex + 2);
                    
                    m_NonInstancedIndices.push_back(baseIndex);
                    m_NonInstancedIndices.push_back(baseIndex + 2);
                    m_NonInstancedIndices.push_back(baseIndex + 3);
                }
            }
        }
    }
    
    counts.vertexCount += static_cast<int>(m_NonInstancedVertices.size());
    counts.indexCount += static_cast<int>(m_NonInstancedIndices.size());
}

void DynamicMesh::CreateBuffers()
{
    // Calculate total buffer sizes needed for all LOD levels
    size_t totalVertices = m_HighLODVertices.size() + m_MediumLODVertices.size() + m_LowLODVertices.size() + m_NonInstancedVertices.size();
    size_t totalIndices = m_HighLODIndices.size() + m_MediumLODIndices.size() + m_LowLODIndices.size() + m_NonInstancedIndices.size();

    // Initialize vertex buffer
    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Dynamic vertex buffer";
    VertBuffDesc.Usage = USAGE_DYNAMIC;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc.Size = sizeof(VertexBuff) * totalVertices;
    m_pDevice->CreateBuffer(VertBuffDesc, nullptr, &m_VertexBuffer);

    // Initialize second vertex buffer
    BufferDesc VertBuffDesc2;
    VertBuffDesc2.Name = "Dynamic vertex buffer 2";
    VertBuffDesc2.Usage = USAGE_DYNAMIC;
    VertBuffDesc2.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc2.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc2.Size = sizeof(VertexBuff2) * totalVertices;
    m_pDevice->CreateBuffer(VertBuffDesc2, nullptr, &m_VertexBuffer2);

    // Initialize third vertex buffer
    BufferDesc VertBuffDesc3;
    VertBuffDesc3.Name = "Dynamic vertex buffer 3";
    VertBuffDesc3.Usage = USAGE_DYNAMIC;
    VertBuffDesc3.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc3.CPUAccessFlags = CPU_ACCESS_WRITE;
    VertBuffDesc3.Size = sizeof(VertexBuff3) * totalVertices;
    m_pDevice->CreateBuffer(VertBuffDesc3, nullptr, &m_VertexBuffer3);

    // Initialize index buffer
    BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Dynamic index buffer";
    IndBuffDesc.Usage = USAGE_DYNAMIC;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    IndBuffDesc.Size = sizeof(Uint32) * totalIndices;
    m_pDevice->CreateBuffer(IndBuffDesc, nullptr, &m_IndexBuffer);
}

void DynamicMesh::PrepareResources()
{
    //if (m_GPUDataInitialized)
    //    return;

    // Update vertex buffer with all LOD levels
    MapHelper<VertexBuff> Vertices(m_pContext, m_VertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    
    // Copy high LOD vertices
    size_t vertexOffset = 0;
    memcpy(Vertices + vertexOffset, m_HighLODVertices.data(), sizeof(VertexBuff) * m_HighLODVertices.size());
    vertexOffset += m_HighLODVertices.size();
    
    // Copy medium LOD vertices
    memcpy(Vertices + vertexOffset, m_MediumLODVertices.data(), sizeof(VertexBuff) * m_MediumLODVertices.size());
    vertexOffset += m_MediumLODVertices.size();
    
    // Copy low LOD vertices
    memcpy(Vertices + vertexOffset, m_LowLODVertices.data(), sizeof(VertexBuff) * m_LowLODVertices.size());
    vertexOffset += m_LowLODVertices.size();

    // Copy non-instanced vertices
    memcpy(Vertices + vertexOffset, m_NonInstancedVertices.data(), sizeof(VertexBuff) * m_NonInstancedVertices.size());
    vertexOffset += m_NonInstancedVertices.size();

    // Update second vertex buffer (initialize with zeros)
    MapHelper<VertexBuff2> Vertices2(m_pContext, m_VertexBuffer2, MAP_WRITE, MAP_FLAG_DISCARD);
    size_t totalVertices = m_HighLODVertices.size() + m_MediumLODVertices.size() + m_LowLODVertices.size() + m_NonInstancedVertices.size();
    memset(Vertices2, 0, sizeof(VertexBuff2) * totalVertices);

    // Update third vertex buffer (initialize with zeros)
    MapHelper<VertexBuff3> Vertices3(m_pContext, m_VertexBuffer3, MAP_WRITE, MAP_FLAG_DISCARD);
    memset(Vertices3, 0, sizeof(VertexBuff3) * totalVertices);

    // Update index buffer with all LOD levels
    MapHelper<Uint32> Indices(m_pContext, m_IndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
    
    // Copy high LOD indices
    size_t indexOffset = 0;
    memcpy(Indices + indexOffset, m_HighLODIndices.data(), sizeof(Uint32) * m_HighLODIndices.size());
    indexOffset += m_HighLODIndices.size();
    
    // Copy medium LOD indices
    memcpy(Indices + indexOffset, m_MediumLODIndices.data(), sizeof(Uint32) * m_MediumLODIndices.size());
    indexOffset += m_MediumLODIndices.size();
    
    // Copy low LOD indices
    memcpy(Indices + indexOffset, m_LowLODIndices.data(), sizeof(Uint32) * m_LowLODIndices.size());
    indexOffset += m_LowLODIndices.size();

    // Copy non-instanced indices
    memcpy(Indices + indexOffset, m_NonInstancedIndices.data(), sizeof(Uint32) * m_NonInstancedIndices.size());
    indexOffset += m_NonInstancedIndices.size();

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

//#define TEST_LOW_LOD_NON_INSTANCED_TERRAIN 1
#define TEST_LOW_LOD_TERRAIN 1

#if 0
// Takes world space coordinates and generates an amalgamation of terrain tiles underneath the coordinates
// with UVs that map to the terrain heightmap
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement)
{
    // TODO: Integrate low, mid, and high LOD terrain
}
#elif defined(TEST_LOW_LOD_NON_INSTANCED_TERRAIN)
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement)
{
    auto& node = Nodes[0];
    
    // Clear existing instances
    node.Instances.clear();
    
    // Create a single instance for the non-instanced terrain
    NodeInstance instance;
    
    // Set the transformation matrix to position the terrain
    // Apply the provided terrain movement to position the terrain in world space
    instance.NodeMatrix = float4x4::Translation(terrainMovement);
    
    // Set texture scaling and offset parameters
    // These will be used in the shader to properly map textures to the terrain
    instance.ScaleX = 1.0f / m_TextureSize.x;
    instance.ScaleY = 1.0f / m_TextureSize.y;
    instance.OffsetX = 0.0f;
    instance.OffsetY = 0.0f;
    
    // Add the instance to the node
    node.Instances.push_back(instance);
    
    m_Mesh = std::make_shared<SF_GLTF::Mesh>();
    m_Mesh->Primitives.emplace_back(m_NonInstancedOffsets.indexCount, m_NonInstancedIndices.size(), m_NonInstancedVertices.size() / 4, 0, float3{}, float3{});
    
    float totalWidth = TERRAIN_MAX_X * m_TileSize.x;
    float totalHeight = TERRAIN_MAX_Y * m_TileSize.y;
    
    node.pMesh = m_Mesh.get();

    m_Mesh->BB.Min = float3{ -totalWidth, -2.0f, -totalHeight * 3.0f };
    m_Mesh->BB.Max = float3{ totalWidth * 2.0f, 10.0f, totalHeight * 4.0f };
}
#elif defined(TEST_LOW_LOD_TERRAIN)
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement, float heightFactor)
{
    auto& node = Nodes[0];

    // Calculate the size of the entire grid of tiles
    float totalWidth = TERRAIN_MAX_X * m_TileSize.x;
    float totalHeight = TERRAIN_MAX_Y * m_TileSize.y;
    
    // Clear existing instances
    node.Instances.clear();
    
    // Calculate how many low LOD tiles we need to cover one terrain section
    // Each low LOD tile covers numBigTiles x numBigTiles area
    int lowLODTilesX = (TERRAIN_MAX_X + numBigTiles.x - 1) / numBigTiles.x;
    int lowLODTilesY = (TERRAIN_MAX_Y + numBigTiles.y - 1) / numBigTiles.y;
    
    // Create 3x5 repeating pattern of the terrain (3 columns, 5 rows)
    for (int repeatX = -1; repeatX <= 1; ++repeatX) {
        for (int repeatY = -3; repeatY <= 3; ++repeatY) {
            // Calculate the offset for this terrain section
            float xOffset = repeatX * totalWidth;
            float zOffset = repeatY * totalHeight;
            
            // Create low LOD tiles to cover this terrain section
            for (int x = 0; x < lowLODTilesX; ++x) {
                for (int y = 0; y < lowLODTilesY; ++y) {
                    // Calculate the actual tile position
                    int2 tile = int2{x * numBigTiles.x, y * numBigTiles.y};
                    
                    // Create instance for this low LOD tile
                    NodeInstance instance;
                    auto translation = float4x4::Translation(float3{
                        xOffset + (float)tile.x * m_TileSize.x,
                        0.0f,
                        zOffset + (float)tile.y * m_TileSize.y
                    });
                    
                    // Calculate texture scale and offset
                    instance.ScaleX = numBigTiles.x / m_TextureSize.x;
                    instance.ScaleY = numBigTiles.y / m_TextureSize.y;
                    
                    // Calculate texture offsets with mirroring in the vertical direction (Y)
                    instance.OffsetX = (float)tile.x / m_TextureSize.x;
                    
                    // Apply mirroring in vertical direction if repeatY is odd
                    if (repeatY % 2 == 0) {
                        instance.OffsetY = (float)tile.y / m_TextureSize.y;
                    } else {
                        // Mirror the Y coordinate by calculating offset from the opposite end
                        instance.OffsetY = ((float)TERRAIN_MAX_Y - (float)tile.y - (float)numBigTiles.y) / m_TextureSize.y;
                        instance.ScaleY = -instance.ScaleY; // Flip the scale for mirroring
                    }
                    
                    instance.PlanetLocation = tile;
                    instance.NodeMatrix = translation;
                    
                    node.Instances.push_back(instance);
                }
            }
        }
    }

    m_Mesh = std::make_shared<SF_GLTF::Mesh>();
    m_Mesh->Primitives.emplace_back(m_LowLODOffsets.indexCount, m_LowLODIndices.size(), m_LowLODVertices.size() / 4, 0, float3{}, float3{});

    // Set the bounding box to encompass the entire 3x5 terrain pattern
    m_Mesh->BB.Min = float3{ -totalWidth, -2.0f, -totalHeight * 3.0f };
    m_Mesh->BB.Max = float3{ totalWidth * 2.0f, 10.0f, totalHeight * 4.0f };

    m_Mesh->BB.Min.y *= heightFactor;
    m_Mesh->BB.Max.y *= heightFactor;

    node.pMesh = m_Mesh.get();
}
#elif defined(TEST_MID_TERRAIN)
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement)
{
    auto& node = Nodes[0];
    
    // Define the 61x101 layout for mid LOD terrain
    const int2 midLODLayout = int2{61, 101};
   
    // Find the center tile coordinate by rounding camera position to the nearest tile grid
    int2 centerTile = int2{
        static_cast<int>(std::round(terrainMovement.x / m_TileSize.x)),
        static_cast<int>(std::round(terrainMovement.z / m_TileSize.y))
    };

    int2 ulTile = centerTile - (midLODLayout / 2);

    // Clear existing instances
    node.Instances.clear();
   
    // Create mid LOD tiles to cover the terrain
    for (int x = 0; x < midLODLayout.x; ++x) {
        for (int y = 0; y < midLODLayout.y; ++y) {
            // Calculate the actual tile position
            int2 tile = ulTile + int2{x, y};
            
            // Create instance for this mid LOD tile
            NodeInstance instance;
            auto translation = float4x4::Translation(float3{
                (float)tile.x * m_TileSize.x,
                0.0f,
                (float)tile.y * m_TileSize.y
            });
            
            // Calculate texture scale and offset
            instance.ScaleX = 1.0f / m_TextureSize.x;
            instance.ScaleY = 1.0f / m_TextureSize.y;
            instance.OffsetX = (float)tile.x / m_TextureSize.x;
            instance.OffsetY = (float)tile.y / m_TextureSize.y;
            instance.PlanetLocation = tile;
            
            instance.NodeMatrix = translation;
            
            node.Instances.push_back(instance);
        }
    }
    
    m_Mesh = std::make_shared<SF_GLTF::Mesh>();
    m_Mesh->Primitives.emplace_back(m_MediumLODOffsets.indexCount, m_MediumLODIndices.size(), m_MediumLODIndices.size() / 4, 0, float3{}, float3{});
    
    float2 upperLeft = float2{(float)ulTile.x * m_TileSize.x, (float)ulTile.y * m_TileSize.y};
    float2 lowerRight = float2{(float)(ulTile.x + highLODLayout.x) * m_TileSize.x, (float)(ulTile.y + highLODLayout.y) * m_TileSize.y};

    m_Mesh->BB.Min = float3{ upperLeft.x, -2.0f, upperLeft.y };
    m_Mesh->BB.Max = float3{ lowerRight.x, 10.0f, lowerRight.y };
    
    node.pMesh = m_Mesh.get();
}
#else
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement)
{
    // Get the node for the high LOD terrain
    auto& node = Nodes[0];

    // Define the 61x101 layout for high LOD terrain
    const int2 highLODLayout = int2{61, 101};

    // Calculate the center tile based on terrain movement
    int2 centerTile = int2{
        static_cast<int>(std::round(terrainMovement.x / m_TileSize.x)),
        static_cast<int>(std::round(terrainMovement.z / m_TileSize.y))
    };

    // Calculate the upper-left tile based on the high LOD layout
    int2 ulTile = centerTile - (highLODLayout / 2);
   
    // Clear existing instances
    node.Instances.clear();
   
    // Create high LOD tiles to cover the terrain
    for (int x = 0; x < highLODLayout.x; ++x) {
        for (int y = 0; y < highLODLayout.y; ++y) {
            // Calculate the actual tile position
            int2 tile = ulTile + int2{x, y};
            
            // Create instance for this high LOD tile
            NodeInstance instance;
            auto translation = float4x4::Translation(float3{
                (float)tile.x * m_TileSize.x,
                0.0f,
                (float)tile.y * m_TileSize.y
            });
            
            // Calculate texture scale and offset
            instance.ScaleX = 1.0f / m_TextureSize.x;
            instance.ScaleY = 1.0f / m_TextureSize.y;
            instance.OffsetX = (float)tile.x / m_TextureSize.x;
            instance.OffsetY = (float)tile.y / m_TextureSize.y;
            instance.PlanetLocation = tile;
            
            instance.NodeMatrix = translation;
            
            node.Instances.push_back(instance);
        }
    }
    
    m_Mesh = std::make_shared<SF_GLTF::Mesh>();
    m_Mesh->Primitives.emplace_back(m_HighLODOffsets.indexCount, m_HighLODIndices.size(), m_HighLODIndices.size() / 4, 0, float3{}, float3{});
  
    float2 upperLeft = float2{(float)ulTile.x * m_TileSize.x, (float)ulTile.y * m_TileSize.y};
    float2 lowerRight = float2{(float)(ulTile.x + highLODLayout.x) * m_TileSize.x, (float)(ulTile.y + highLODLayout.y) * m_TileSize.y};

    m_Mesh->BB.Min = float3{ upperLeft.x, -2.0f, upperLeft.y };
    m_Mesh->BB.Max = float3{ lowerRight.x, 10.0f, lowerRight.y };
    
    node.pMesh = m_Mesh.get();
}

#endif

float DynamicMesh::sampleTerrainBicubic(const TerrainData& terrain, float2 tilePosition, float heightFactor)
{
    // Bicubic sampling helper function
    auto sampleBicubic = [](float v) {
        float4 n = float4{1.0f, 2.0f, 3.0f, 4.0f};
        n = n - float4(v);
        float4 s;
        s.x = n.x * n.x * n.x;
        s.y = n.y * n.y * n.y; 
        s.z = n.z * n.z * n.z;
        s.w = n.w * n.w * n.w;
        float4 o;
        o.x = s.x;
        o.y = s.y - 4.0f * s.x;
        o.z = s.z - 4.0f * s.y + 6.0f * s.x;
        o.w = 6.0f - o.x - o.y - o.z;
        return o;
    };

    // Convert UV coordinates to texture space
    float2 st = tilePosition;
    st += float2(bicubicOffset);

    // Get fractional and integer parts
    float2 fxy;
    fxy.x = st.x - floor(st.x);
    fxy.y = st.y - floor(st.y); 
    st.x -= fxy.x;
    st.y -= fxy.y;

    // Get cubic weights
    float4 xcubic = sampleBicubic(fxy.x);
    float4 ycubic = sampleBicubic(fxy.y);

    // Calculate sample positions
    float4 c;
    c.x = st.x - 0.5f;
    c.y = st.x + 1.5f;
    c.z = st.y - 0.5f;
    c.w = st.y + 1.5f;

    float4 s;
    s.x = xcubic.x + xcubic.z;
    s.y = xcubic.y + xcubic.w;
    s.z = ycubic.x + ycubic.z;
    s.w = ycubic.y + ycubic.w;

    float4 offset;
    offset.x = c.x + xcubic.y/s.x + xcubic.w/s.x;
    offset.y = c.y + xcubic.y/s.y + xcubic.w/s.y;
    offset.z = c.z + ycubic.y/s.z + ycubic.w/s.z;
    offset.w = c.w + ycubic.y/s.w + ycubic.w/s.w;

    // Sample using linear interpolation
    float sample0 = sampleTerrainLinear(terrain, float2(offset.x, offset.z), heightFactor);
    float sample1 = sampleTerrainLinear(terrain, float2(offset.y, offset.z), heightFactor);
    float sample2 = sampleTerrainLinear(terrain, float2(offset.x, offset.w), heightFactor);
    float sample3 = sampleTerrainLinear(terrain, float2(offset.y, offset.w), heightFactor);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    // Final bicubic blend
    float bottom = sample3 * (1.0f - sx) + sample2 * sx;
    float top = sample1 * (1.0f - sx) + sample0 * sx;
    return top * (1.0f - sy) + bottom * sy;
}

float DynamicMesh::sampleTerrainLinearNormalizedUV(const TerrainData& terrain, float2 uv, float heightFactor)
{
    float2 tilePosition = uv * m_TextureSize;
    return sampleTerrainLinear(terrain, tilePosition, heightFactor);
}

float DynamicMesh::sampleTerrainLinear(const TerrainData& terrain, float2 tilePosition, float heightFactor)
{
    // terrain is actually (m_TextureSize.x + 1, m_TextureSize.y + 1)
    // terrain represents the bounds of each individual tile
    // e.g. 0,0, 0,1, 1,0, 1,1 are the bounds of the first tile (0,0)
    // 1,0, 1,1, 2,0, 2,1 are the second tile (1,0)

    // This function returns the center height of the requested tile
    // Get the tile bounds indices

    // Wrap coordinates to handle infinite terrain
    // Mirror coordinates by reflecting across texture boundaries
    // Convert float coordinates to array indices by rounding down

    tilePosition *= terrain.textureScale;

    float2 wrappedPos;
    wrappedPos.x = tilePosition.x < 0 ? -tilePosition.x : 
                   tilePosition.x >= terrain.textureSize.x * terrain.textureScale.x ? 2*terrain.textureSize.x * terrain.textureScale.x - tilePosition.x : 
                   tilePosition.x;
    wrappedPos.y = tilePosition.y < 0 ? -tilePosition.y :
                   tilePosition.y >= terrain.textureSize.y * terrain.textureScale.y ? 2*terrain.textureSize.y * terrain.textureScale.y - tilePosition.y :
                   tilePosition.y;

    float fx = wrappedPos.x - floor(wrappedPos.x);
    float fy = wrappedPos.y - floor(wrappedPos.y);                   

    // Convert to integer indices for array access
    int x = static_cast<int>(floor(wrappedPos.x));
    int y = static_cast<int>(floor(wrappedPos.y));

    // Sample the center point of the tile by averaging the 4 corners
    float h1 = terrain.heightmap[y * (terrain.textureSize.x * terrain.textureScale.x) + x];
    float h2 = terrain.heightmap[y * (terrain.textureSize.x * terrain.textureScale.x) + (x + 1)];
    float h3 = terrain.heightmap[(y + 1) * (terrain.textureSize.x * terrain.textureScale.x) + x];
    float h4 = terrain.heightmap[(y + 1) * (terrain.textureSize.x * terrain.textureScale.x) + (x + 1)];

    float top = h1 * (1.0f - fx) + h2 * fx;
    float bottom = h3 * (1.0f - fx) + h4 * fx;
    float height = top * (1.0f - fy) + bottom * fy;     

    return height * heightFactor;
}

float DynamicMesh::sampleTerrain(const TerrainData& terrain, float2 tilePosition, float heightFactor)
{
    return sampleTerrainLinear(terrain, tilePosition, heightFactor);
}

// Takes the bounding rectangle of a model produces a rotation matrix that aligns the y-axis with the terrain slope
// Think buildings and vehicles aligned properly with the terrain
float3 DynamicMesh::levelPlane(float2 ul, float2 br, const TerrainData& terrain, float4x4* outTerrainSlope, float heightFactor, float waterHeight)
{
    // Sample heights at the four corners and center
    float2 center = (ul + br) * 0.5f;

    float2 ulHeightMap = (float2{ul.x, ul.y} / m_TileSize) - float2{0.5f, 0.5f};
    float2 brHeightMap = (float2{br.x, br.y} / m_TileSize) - float2{0.5f, 0.5f};
    float2 centerHeightMap = (float2{center.x, center.y} / m_TileSize) - float2{0.5f, 0.5f};
    
    float3 corners[5] = {
        float3(ul.x, sampleTerrain(terrain, ulHeightMap, heightFactor), ul.y),      // Upper Left
        float3(br.x, sampleTerrain(terrain, float2(brHeightMap.x, ulHeightMap.y), heightFactor), ul.y),  // Upper Right
        float3(ul.x, sampleTerrain(terrain, float2(ulHeightMap.x, brHeightMap.y), heightFactor), br.y),  // Lower Left
        float3(br.x, sampleTerrain(terrain, brHeightMap, heightFactor), br.y),      // Lower Right
        float3(center.x, sampleTerrain(terrain, centerHeightMap, heightFactor), center.y)  // Center
    };

    // Ensure all corner heights are above water level
    for (int i = 0; i < 5; ++i) {
        corners[i].y = std::max(corners[i].y, waterHeight * heightFactor);
    }    

    // Calculate normal using height differences
    float3 v1 = float3(br.x - ul.x, corners[1].y - corners[0].y, 0.0f);
    float3 v2 = float3(0.0f, corners[2].y - corners[0].y, br.y - ul.y);
    
    float3 normal = normalize(cross(v2, v1));
    if (normal.y < 0) {
        normal = -normal;  // Ensure normal points upward
    }

    // Create rotation matrix that aligns y-axis with normal
    float3 up(0.0f, 1.0f, 0.0f);
    float3 axis = cross(up, normal);
    float angle = acos(dot(up, normal));

    if (outTerrainSlope)
    {
        if (length(axis) < 0.0001f)
        {
            // Normal is nearly parallel to up vector, no rotation needed
            *outTerrainSlope = float4x4::Identity();
        }
        else
        {
            axis = normalize(axis);
            QuaternionF rotation = QuaternionF::RotationFromAxisAngle(axis, angle);
            *outTerrainSlope = rotation.ToMatrix();
        }
    }

    return (corners[0] + corners[1] + corners[2] + corners[3] + corners[4]) / 5.0f;
}

float DynamicMesh::GetHeightAtTerrain(float2 position, const TerrainData& terrain, float heightFactor)
{
    float2 tilePosition = position / m_TileSize;
    return sampleTerrainLinear(terrain, tilePosition, heightFactor);
}

void DynamicMesh::SetTerrainItems(const TerrainItems& terrainItems, const TerrainData& terrain, float heightFactor, float waterHeight)
{
    ClearTerrainItems();

    // Add the Sphere node first (always present)
    auto sphereIt = std::find_if(m_Model->Nodes.begin(), m_Model->Nodes.end(), [](const auto& node) {
        return std::equal(node.Name.begin(), node.Name.end(), "Planet", "Planet" + strlen("Planet"),
                        [](char a, char b) { return tolower(a) == tolower(b); });
    });

    auto planeIt = std::find_if(m_Model->Nodes.begin(), m_Model->Nodes.end(), [](const auto& node) {
        return std::equal(node.Name.begin(), node.Name.end(), "Plane-2", "Plane-2" + strlen("Plane-2"),
                        [](char a, char b) { return tolower(a) == tolower(b); });
    });

    // Find the Rover in terrainItems
    auto roverIt = std::find_if(terrainItems.begin(), terrainItems.end(), [](const auto& item) {
        return std::equal(item.name.begin(), item.name.end(), "Rover", "Rover" + strlen("Rover"),
                        [](char a, char b) { return tolower(a) == tolower(b); });
    });
    
    // Store the rover position if found
    float3 roverPosition;
    bool roverFound = false;
    
    if (roverIt != terrainItems.end()) {
        roverPosition = float3(roverIt->tilePosition.x * m_TileSize.x, 0.0f, roverIt->tilePosition.y * m_TileSize.y);
        roverFound = true;
    }

    assert(roverFound);

#if 0
    if (sphereIt != m_Model->Nodes.end())
    {
        Node sphereNode{static_cast<int>(Nodes.size())};
        sphereNode.Name = sphereIt->Name;
        sphereNode.pMesh = sphereIt->pMesh;
        sphereNode.Translation = float3{0.0f, 0.0f, 0.0f};
        sphereNode.Rotation = Quaternion<float>{0.0f, 0.0f, 0.0f, 1.0f};

        // Scale the sphere to match the world size
        float worldScale = TERRAIN_MAX_X * m_TileSize.x;
        float sphereScale = worldScale;

        NodeInstance sphereInstance;
        // Position the sphere at the center of the world, but below the Rover
        float3 worldCenter = float3{roverPosition.x, -2.0f * sphereScale, roverPosition.z};  // Changed Y coordinate
        float4x4 translationMatrix = float4x4::Translation(worldCenter);
        float4x4 scaleMatrix = float4x4::Scale(sphereScale, sphereScale, sphereScale);

        sphereNode.Matrix = scaleMatrix * translationMatrix;

        Nodes.emplace_back(sphereNode);
        Scenes[0].LinearNodes.push_back(&Nodes.back());
        Scenes[0].RootNodes.push_back(&Nodes.back());
    }
#endif

#if 0
    if (planeIt != m_Model->Nodes.end())
    {
        Node planeNode{static_cast<int>(Nodes.size())};
        planeNode.Name = planeIt->Name;
        planeNode.pMesh = planeIt->pMesh;
        planeNode.Translation = float3{0.0f, 0.0f, 0.0f};
        planeNode.Rotation = Quaternion<float>{0.0f, 0.0f, 0.0f, 1.0f};

        NodeInstance planeInstance;
        float3 worldCenter = float3{0.0f, 0.0f, 0.0f};
        float4x4 translationMatrix = float4x4::Translation(worldCenter);
        float4x4 scaleMatrix = float4x4::Scale(TERRAIN_MAX_X * m_TileSize.x, 1.0f, TERRAIN_MAX_Y * m_TileSize.y);
        planeNode.Matrix = scaleMatrix * translationMatrix;

        Nodes.emplace_back(planeNode);
        Scenes[0].LinearNodes.push_back(&Nodes.back());
        Scenes[0].RootNodes.push_back(&Nodes.back());
    }
#endif

    // Continue with the rest of the terrain items
    for (const auto& item : terrainItems)
    {
        auto it = std::find_if(m_Model->Nodes.begin(), m_Model->Nodes.end(), [&item](const auto& node) {
            return std::equal(node.Name.begin(), node.Name.end(), item.name.begin(), item.name.end(),
                            [](char a, char b) { return tolower(a) == tolower(b); });
        });

        assert(it != m_Model->Nodes.end());

        if (it != m_Model->Nodes.end())
        {
            // Special cases that always get their own node
            bool isSpecialCase = (item.name == "Rover" || item.name == "SF_Ramp" || item.name == "Endurium");
            
            Node* targetNode = nullptr;
            
            if (!isSpecialCase)
            {
                // Check if a node with this name already exists
                auto existingNodeIt = std::find_if(Nodes.begin() + 1, Nodes.end(), [&item](const Node& node) {
                    return std::equal(node.Name.begin(), node.Name.end(), item.name.begin(), item.name.end(),
                                    [](char a, char b) { return tolower(a) == tolower(b); });
                });
                
                if (existingNodeIt != Nodes.end())
                {
                    targetNode = &(*existingNodeIt);
                }
            }
            
            // Create new node if needed
            if (targetNode == nullptr)
            {
                Nodes.emplace_back(static_cast<int>(Nodes.size()));
                targetNode = &Nodes.back();
                
                targetNode->Name = it->Name;
                targetNode->pMesh = it->pMesh;
                targetNode->Translation = float3{0.0f, 0.0f, 0.0f};
                targetNode->Rotation = Quaternion<float>{0.0f, 0.0f, 0.0f, 1.0f};
                
                Scenes[0].LinearNodes.push_back(targetNode);
                Scenes[0].RootNodes.push_back(targetNode);
            }

            NodeInstance ni;
            float4x4 terrainSlope = float4x4::Identity();
            float4x4* terrainSlopePtr = item.alignToTerrain ? &terrainSlope : nullptr;

            float2 modelSize = float2{it->pMesh->BB.Max.x - it->pMesh->BB.Min.x, it->pMesh->BB.Max.z - it->pMesh->BB.Min.z};

            //float2 ul = float2{-0.5f, -0.5f};
            //float2 br = float2{0.5f, 0.5f};
            float2 ul = float2{-modelSize.x / 2.0f, -modelSize.y / 2.0f};
            float2 br = float2{modelSize.x / 2.0f, modelSize.y / 2.0f};

            // Rotate ul and br by the item rotation quaternion
            float3 ul3 = float3(ul.x, 0.0f, ul.y);
            float3 br3 = float3(br.x, 0.0f, br.y);
            
            ul3 = item.rotation.RotateVector(ul3);
            br3 = item.rotation.RotateVector(br3);
            
            ul = float2(ul3.x, ul3.z);
            br = float2(br3.x, br3.z);

            // Translate ul and br by the tile position
            ul += (item.tilePosition + float2{0.5f, 0.5f}) * m_TileSize;
            br += (item.tilePosition + float2{0.5f, 0.5f}) * m_TileSize;

            float3 worldOffset = levelPlane(ul, br, terrain, terrainSlopePtr, heightFactor, waterHeight);

            worldOffset += item.worldOffset;

            float4x4 translationMatrix = float4x4::Translation(worldOffset);
            float4x4 rotationMatrix = item.rotation.ToMatrix();

            float4x4 scaleMatrix = float4x4::Scale(item.scale, item.scale, item.scale);
            
            // Special handling for items that use Matrix directly instead of instances
            if (isSpecialCase) {
                targetNode->Matrix = scaleMatrix * rotationMatrix * terrainSlope * translationMatrix;
                // Don't add any instances for special cases
            } else {
                // For all other items, use the instance approach
                ni.NodeMatrix = scaleMatrix * rotationMatrix * terrainSlope * translationMatrix;
                ni.ScaleX = 1.0f;
                ni.ScaleY = 1.0f;
                ni.OffsetX = 0.0f;
                ni.OffsetY = 0.0f;
                targetNode->Instances.push_back(ni);
            }

            // Check if this item has any light children (only for newly created nodes)
            if (targetNode == &Nodes.back()) // Only process lights for newly created nodes
            {
                bool hasLights = false;
                for (const auto& child : it->Children)
                {
                    if (child->pLight != nullptr)
                    {
                        hasLights = true;
                        break;
                    }
                }

                if(hasLights)
                {
                    Node* parentNode = targetNode;
                    
                    for (const auto& child : it->Children)
                    {
                        if (child->pLight != nullptr)
                        {
                            Node lightNode = *child;
                            auto& newNode = Nodes.emplace_back(lightNode);
                            newNode.Index = Nodes.size() - 1;
                            
                            newNode.Parent = parentNode;
                            parentNode->Children.push_back(&newNode);
                            
                            Scenes[0].LinearNodes.push_back(&newNode);

                            lights.push_back(&newNode);
                        }
                    }
                }
            }
        }
    }
}

void DynamicMesh::ClearTerrainItems()
{
    Scenes[0].LinearNodes.erase(Scenes[0].LinearNodes.begin() + 1, Scenes[0].LinearNodes.end());
    Scenes[0].RootNodes.erase(Scenes[0].RootNodes.begin() + 1, Scenes[0].RootNodes.end());
    Nodes.erase(Nodes.begin() + 1, Nodes.end());

    lights.clear();
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