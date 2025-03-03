#include "DynamicMesh.hpp"
#include "MapHelper.hpp"

#include <Windows.h>

extern float bicubicOffset;

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
    const int numTiles = 8;
    const int numVerticesPerRow = numTiles + 1;
    const int numVertices = numVerticesPerRow * numVerticesPerRow;
    m_TileSize = float2{ width, height };
    m_TextureSize = textureSize;

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

    m_TileBB.Min = minBB;
    m_TileBB.Max = maxBB;

    float3 baseMinBB = minBB;
    float3 baseMaxBB = maxBB;

    for (int i = 0; i < numBigTiles; ++i)
    {
        for (int j = 0; j < numBigTiles; ++j)
        {
            NodeInstance ni;
            ni.NodeMatrix = float4x4::Translation(i * width - offsetX, 0.0f, j * height - offsetY);
            ni.ScaleX = 1.0f / (float)textureSize.x;
            ni.ScaleY = 1.0f / (float)textureSize.y;
            ni.OffsetX = i / (float)textureSize.x;
            ni.OffsetY = j / (float)textureSize.y;
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

// Takes world space coordinates and generates an amalgamation of terrain tiles underneath the coordinates
// with UVs that map to the terrain heightmap
void DynamicMesh::ReplaceTerrain(const float3& terrainMovement)
{
    auto& node = Nodes[0];

    // Calculate the size of the entire grid of tiles
    float totalWidth = numBigTiles * m_TileSize.x;
    float totalHeight = numBigTiles * m_TileSize.y;
    
    // Find the center tile coordinate by rounding camera position to the nearest tile grid
    int2 centerTile = int2{
        static_cast<int>(std::round(terrainMovement.x / m_TileSize.x)),
        static_cast<int>(std::round(terrainMovement.z / m_TileSize.y))
    };

    int2 ulTile = centerTile - int2{numBigTiles / 2, numBigTiles / 2};

    float3 baseMinBB = m_TileBB.Min;
    float3 baseMaxBB = m_TileBB.Max;

    float3 minBB = float3{ std::numeric_limits<float>::max() };
    float3 maxBB = float3{ std::numeric_limits<float>::lowest() };    

    size_t instanceIdx = 0;
    for (int i = 0; i < numBigTiles; ++i)
    {
        for (int j = 0; j < numBigTiles; ++j)
        {
            int2 tile = ulTile + int2{i, j};
            
            // Update the instance matrix while preserving UV mappings
            auto& instance = node.Instances[instanceIdx];

            float3 translation = float3{
                (float)tile.x * m_TileSize.x,
                0.0f,
                (float)tile.y * m_TileSize.y
            };

            instance.NodeMatrix = float4x4::Translation(translation);
            
            // Update UV scale/offset to match the new world position
            instance.ScaleX = 1.0f / m_TextureSize.x;
            instance.ScaleY = 1.0f / m_TextureSize.y;
            instance.OffsetX = (float)tile.x / m_TextureSize.x;
            instance.OffsetY = (float)tile.y / m_TextureSize.y;
            instance.PlanetLocation = tile;

            float3 minTranslatedPos = baseMinBB * instance.NodeMatrix;
            float3 maxTranslatedPos = baseMaxBB * instance.NodeMatrix;

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
            
            instanceIdx++;
        }
    }

    float yValues[] = { -2.0f, 10.0f };

    for (float y : yValues)
    {
        float3 minTranslatedPos = minBB * float4x4::Translation(0.0f, y, 0.0f);
        float3 maxTranslatedPos = maxBB * float4x4::Translation(0.0f, y, 0.0f);

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
}

float DynamicMesh::sampleTerrainBicubic(const TerrainData& terrain, float2 tilePosition)
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
    float sample0 = sampleTerrainLinear(terrain, float2(offset.x, offset.z));
    float sample1 = sampleTerrainLinear(terrain, float2(offset.y, offset.z));
    float sample2 = sampleTerrainLinear(terrain, float2(offset.x, offset.w));
    float sample3 = sampleTerrainLinear(terrain, float2(offset.y, offset.w));

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    // Final bicubic blend
    float bottom = sample3 * (1.0f - sx) + sample2 * sx;
    float top = sample1 * (1.0f - sx) + sample0 * sx;
    return top * (1.0f - sy) + bottom * sy;
}

float DynamicMesh::sampleTerrainLinearNormalizedUV(const TerrainData& terrain, float2 uv)
{
    float2 tilePosition = uv * m_TextureSize;
    return sampleTerrainLinear(terrain, tilePosition);
}

float DynamicMesh::sampleTerrainLinear(const TerrainData& terrain, float2 tilePosition)
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

    return height;
}

float DynamicMesh::sampleTerrain(const TerrainData& terrain, float2 tilePosition)
{
    return sampleTerrainLinear(terrain, tilePosition);
}

// Takes the bounding rectangle of a model produces a rotation matrix that aligns the y-axis with the terrain slope
// Think buildings and vehicles aligned properly with the terrain
float3 DynamicMesh::levelPlane(float2 ul, float2 br, const TerrainData& terrain, float4x4* outTerrainSlope)
{
    // Sample heights at the four corners and center
    float2 center = (ul + br) * 0.5f;

    float2 ulHeightMap = (float2{ul.x, ul.y} / m_TileSize) - float2{0.5f, 0.5f};
    float2 brHeightMap = (float2{br.x, br.y} / m_TileSize) - float2{0.5f, 0.5f};
    float2 centerHeightMap = (float2{center.x, center.y} / m_TileSize) - float2{0.5f, 0.5f};
    
    float3 corners[5] = {
        float3(ul.x, sampleTerrain(terrain, ulHeightMap), ul.y),      // Upper Left
        float3(br.x, sampleTerrain(terrain, float2(brHeightMap.x, ulHeightMap.y)), ul.y),  // Upper Right
        float3(ul.x, sampleTerrain(terrain, float2(ulHeightMap.x, brHeightMap.y)), br.y),  // Lower Left
        float3(br.x, sampleTerrain(terrain, brHeightMap), br.y),      // Lower Right
        float3(center.x, sampleTerrain(terrain, centerHeightMap), center.y)  // Center
    };

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

void DynamicMesh::SetTerrainItems(const TerrainItems& terrainItems, const TerrainData& terrain)
{
    ClearTerrainItems();

    for (const auto& item : terrainItems)
    {
        auto it = std::find_if(m_Model->Nodes.begin(), m_Model->Nodes.end(), [&item](const auto& node) {
            return std::equal(node.Name.begin(), node.Name.end(), item.name.begin(), item.name.end(),
                            [](char a, char b) { return tolower(a) == tolower(b); });
        });

        assert(it != m_Model->Nodes.end());

        if (it != m_Model->Nodes.end())
        {
            Node ourNode{static_cast<int>(Nodes.size())};

            ourNode.Name = it->Name;
            ourNode.pMesh = it->pMesh;
            ourNode.Translation = float3{0.0f, 0.0f, 0.0f};
            ourNode.Rotation = Quaternion<float>{0.0f, 0.0f, 0.0f, 1.0f};

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

            float3 worldOffset = levelPlane(ul, br, terrain, terrainSlopePtr);

            //char debugStr[256];
            //sprintf_s(debugStr, "WorldOffset for %s (node %d): %.2f, %.2f, %.2f\n", item.name.c_str(), ourNode.Index, worldOffset.x, worldOffset.y, worldOffset.z);
            //OutputDebugStringA(debugStr);

            float val = sampleTerrainLinear(terrain, item.tilePosition);
            //worldOffset.x = (item.tilePosition.x + 0.5f) * m_TileSize.x;
            //worldOffset.z = (item.tilePosition.y + 0.5f) * m_TileSize.y;

#if 0
            if(item.name == "Rover")
            {
                std::vector<float> yValues;
                std::vector<float> sampledHeights;
                std::vector<float> originalValues;

                // Sample in 1/20th increments
                for (float y = -3.0f; y <= 3.0f; y += 0.05f)  // 0.05 = 1/20
                {
                    int x = static_cast<int>(floor(item.tilePosition.x));
                    int ogY = static_cast<int>(floor(y + item.tilePosition.y));
                    float h1 = terrain[ogY * (m_TextureSize.x + 1) + x];
                    originalValues.push_back(h1);

                    yValues.push_back(y + item.tilePosition.y);
                    float sampledHeight = sampleTerrain(terrain, item.tilePosition + float2{0.0f, y}, nullptr);
                    sampledHeights.push_back(sampledHeight);
                }

                // Create JSON string
                std::ostringstream jsonStream;
                jsonStream << "{ \"values\": [";
                for (size_t i = 0; i < yValues.size(); ++i)
                {
                    jsonStream << "[" << yValues[i] << ", " << sampledHeights[i] << ", " << originalValues[i] << "]";
                    if (i < yValues.size() - 1) jsonStream << ", ";
                }
                jsonStream << "] }";

                std::string jsonString = jsonStream.str();

                printf("%s\n", jsonString.c_str());

                _exit(0);
            }
#endif

            worldOffset += item.worldOffset;

            //worldOffset.y = val;

            float4x4 translationMatrix = float4x4::Translation(worldOffset);
            float4x4 rotationMatrix = item.rotation.ToMatrix();
            ni.NodeMatrix = rotationMatrix * terrainSlope * translationMatrix;

            ni.ScaleX = 1.0f;
            ni.ScaleY = 1.0f;
            ni.OffsetX = 0.0f;
            ni.OffsetY = 0.0f;
            ourNode.Instances.push_back(ni);

            Nodes.emplace_back(ourNode);
            Scenes[0].LinearNodes.push_back(&Nodes.back());
            Scenes[0].RootNodes.push_back(&Nodes.back());

            if(item.name == "Rover")
            {
                // Reserve space for lights to prevent reallocation
                Lights.reserve(Lights.size() + 2);
                
                // Add headlights to the rover
                SF_GLTF::Light headlight1;
                headlight1.Type = SF_GLTF::Light::TYPE::SPOT;
                headlight1.Color = float3(1.0f, 1.0f, 0.9f);      // Slightly warm white
                headlight1.Intensity = 50.0f;
                headlight1.Range = 100.0f;
                headlight1.InnerConeAngle = PI_F / 12.0f;  // 15 degrees for tight inner beam
                headlight1.OuterConeAngle = PI_F / 6.0f;   // 30 degrees for wider outer falloff
                
                // Create a node for the right headlight
                SF_GLTF::Node headlightNode1 = Nodes.back();
                SF_GLTF::NodeInstance ni1;
                // Use the rover's rotation and position for the headlight
                float4x4 roverRotation = item.rotation.ToMatrix();
                float4x4 roverTranslation = float4x4::Translation(worldOffset);
                // Position the right headlight relative to the rover
                float4x4 rightHeadlightOffset = float4x4::Translation(float3(0.5f, 0.5f, 0.0f));
                ni1.NodeMatrix = roverRotation * rightHeadlightOffset * roverTranslation;
                Lights.push_back(headlight1);
                
                headlightNode1.pLight = &Lights.back();
                headlightNode1.Instances.push_back(ni1);
                Nodes.push_back(headlightNode1);
                Scenes[0].LinearNodes.push_back(&Nodes.back());
                dynamicLights.push_back(&Nodes.back());
                
                SF_GLTF::Light headlight2 = headlight1;
                SF_GLTF::Node headlightNode2 = Nodes.back();
                SF_GLTF::NodeInstance ni2;
                // Position the left headlight relative to the rover
                float4x4 leftHeadlightOffset = float4x4::Translation(float3(-0.5f, 0.5f, 0.0f));
                ni2.NodeMatrix = roverRotation * leftHeadlightOffset * roverTranslation;
                Lights.push_back(headlight2);
                headlightNode2.pLight = &Lights.back();
                headlightNode2.Instances.push_back(ni2);
                Nodes.push_back(headlightNode2);
                Scenes[0].LinearNodes.push_back(&Nodes.back());
                dynamicLights.push_back(&Nodes.back());
            }
        }
    }
}

void DynamicMesh::ClearTerrainItems()
{
    Scenes[0].LinearNodes.erase(Scenes[0].LinearNodes.begin() + 1, Scenes[0].LinearNodes.end());
    Scenes[0].RootNodes.erase(Scenes[0].RootNodes.begin() + 1, Scenes[0].RootNodes.end());
    Nodes.erase(Nodes.begin() + 1, Nodes.end());

    Lights.clear();
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