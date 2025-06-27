#pragma once

#include "../pbr/SF_GLTFLoader.hpp"
#include "BasicTypes.h"

namespace Diligent
{

namespace SF_GLTF
{

struct TerrainData
{
    const std::vector<float>& heightmap;
    int2 textureSize;
    float2 textureScale;
};

struct TerrainItem
{
    std::string name;
    float2 tilePosition;
    float3 worldOffset;
    Quaternion<float> rotation;
    bool alignToTerrain = false;
    float3 scale = {1.0f, 1.0f, 1.0f};
};

typedef std::vector<TerrainItem> TerrainItems;

class DynamicMesh : public Model
{
public:
    DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<SF_GLTF::Model>& model);
    ~DynamicMesh();

    void PrepareResources();

    void CreateBuffers();

    void GeneratePlanes(float width, float height, float tileHeight, float2 textureSize);

    const std::vector<SF_GLTF::Material>& GetMaterials() const override
    {
        return m_Model->GetMaterials();
    }

    std::vector<SF_GLTF::Material>& GetMaterials() override
    {
        return m_Model->GetMaterials();
    }

    void ReplaceTerrain(const float3& terrainMovement, float heightFactor);
    void SetTerrainItems(const TerrainItems& terrainItems, const TerrainData& terrain, float heightFactor, float waterHeight);
    void ClearTerrainItems();

    float GetHeightAtTerrain(float2 position, const TerrainData& terrain, float heightFactor);
    float GetHeightAtTerrainWithoutTiling(float2 position, const TerrainData& terrain);

    virtual bool CompatibleWithTransforms(const SF_GLTF::ModelTransforms& Transforms) const override;

    virtual void ComputeTransforms(Uint32           SceneIndex,
                                    SF_GLTF::ModelTransforms& Transforms,
                                    const float4x4& RootTransform = float4x4::Identity(),
                                    Int32            AnimationIndex = -1,
                                    float            Time = 0) const override;

    virtual BoundBox ComputeBoundingBox(Uint32 SceneIndex, const SF_GLTF::ModelTransforms& Transforms, const SF_GLTF::ModelTransforms* DynamicTransforms) const override;

    virtual void SetVertexBuffersForNode(IDeviceContext* pCtx, const Node* node) const override;
    virtual Uint32 GetFirstIndexLocationForNode(const Node* node) const override;
    virtual Uint32 GetBaseVertexForNode(const Node* node) const override;
    virtual Uint32 GetNumVertexAttributesForNode(const Node* node) const override;
    virtual const VertexAttributeDesc GetVertexAttributeForNode(const Node* node, size_t Idx) const override;
    virtual bool IsVertexAttributeEnabledForNode(const Node* node, Uint32 AttribId) const override;

    std::vector<const SF_GLTF::Node*> lights;

private:

    const int2 numBigTiles = { 64, 96 };
    float2 m_TileSize = { 8.0f, 4.0f };
    float2 m_TextureSize = { 1024.0f, 1024.0f };
    
    void InitializeVertexAndIndexData();

    float sampleTerrain(const TerrainData& terrain, float2 tilePosition, float heightFactor);
    float sampleTerrainLinear(const TerrainData& terrain, float2 tilePosition, float heightFactor);
    float sampleTerrainLinearNormalizedUV(const TerrainData& terrain, float2 uv, float heightFactor);
    float sampleTerrainBicubic(const TerrainData& terrain, float2 tilePosition, float heightFactor);
    float3 levelPlane(float2 ul, float2 br, const TerrainData& terrain, float4x4* outTerrainSlope, float heightFactor, float waterHeight = -1000.0f);

    struct VertexIndexCounts
    {
        int vertexCount;
        int indexCount;
    };

    VertexIndexCounts m_HighLODOffsets;
    VertexIndexCounts m_MediumLODOffsets;
    VertexIndexCounts m_LowLODOffsets;
    VertexIndexCounts m_NonInstancedOffsets;

    void generateHighLODMesh(VertexIndexCounts& counts, int numQuadsPerTile, float tileHeight);
    void generateMediumLODMesh(VertexIndexCounts& counts, float tileHeight);
    void generateLowLODMesh(VertexIndexCounts& counts, float tileHeight);
    void generateNonInstancedMesh(VertexIndexCounts& counts, float tileHeight);

    struct VertexBuff
    {
        float posX;
        float posY;
        float posZ;
        float normalX;
        float normalY;
        float normalZ;
        float texU0;
        float texV0;
    };

    struct VertexBuff2
    {
        float jointX;
        float jointY;
        float jointZ;
        float jointW;
        float weightX;
        float weightY;
        float weightZ;
        float weightW;
    };

    struct VertexBuff3
    {
        float texU1;
        float texV1;
        float padding0;
        float padding1;
        float padding2;
        float padding3;
        float padding4;
        float padding5;
    };

    std::vector<VertexBuff> m_HighLODVertices;
    std::vector<Uint32> m_HighLODIndices;
    std::vector<VertexBuff> m_MediumLODVertices;
    std::vector<Uint32> m_MediumLODIndices;
    std::vector<VertexBuff> m_LowLODVertices;
    std::vector<Uint32> m_LowLODIndices;

    std::vector<VertexBuff> m_NonInstancedVertices;
    std::vector<Uint32> m_NonInstancedIndices;

    RefCntAutoPtr<IBuffer> m_VertexBuffer;
    RefCntAutoPtr<IBuffer> m_VertexBuffer2;
    RefCntAutoPtr<IBuffer> m_VertexBuffer3;
    RefCntAutoPtr<IBuffer> m_IndexBuffer;
    std::shared_ptr<SF_GLTF::Model> m_Model;
    IRenderDevice* m_pDevice;
    IDeviceContext* m_pContext; 

    std::shared_ptr<SF_GLTF::Mesh> m_Mesh;
    BoundBox m_TileBB;

    bool m_GPUDataInitialized = false;
};

} // namespace SF_GLTF

} // namespace Diligent