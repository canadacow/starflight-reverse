#pragma once

#include "../pbr/SF_GLTFLoader.hpp"
#include "BasicTypes.h"

namespace Diligent
{

namespace SF_GLTF
{

class DynamicMesh : public Model
{
public:
    DynamicMesh(IRenderDevice* pDevice, IDeviceContext* pContext, const std::shared_ptr<SF_GLTF::Model>& model);
    ~DynamicMesh();

    void PrepareResources();

    void CreateBuffers();

    void GeneratePlane(float width, float height, float tileHeight);

    const std::vector<SF_GLTF::Material>& GetMaterials() const override
    {
        return m_Model->GetMaterials();
    }

    std::vector<SF_GLTF::Material>& GetMaterials() override
    {
        return m_Model->GetMaterials();
    }


    virtual bool CompatibleWithTransforms(const SF_GLTF::ModelTransforms& Transforms) const override;

    virtual void ComputeTransforms(Uint32           SceneIndex,
                                    SF_GLTF::ModelTransforms& Transforms,
                                    const float4x4& RootTransform = float4x4::Identity(),
                                    Int32            AnimationIndex = -1,
                                    float            Time = 0) const override;

    virtual BoundBox ComputeBoundingBox(Uint32 SceneIndex, const SF_GLTF::ModelTransforms& Transforms, const SF_GLTF::ModelTransforms* DynamicTransforms) const override;

private:

    void InitializeVertexAndIndexData();

    struct VertexBuff
    {
        float posX;
        float posY;
        float posZ;
        float normalX;
        float normalY;
        float normalZ;
        float texU;
        float texV;
    };

    RefCntAutoPtr<IBuffer> m_VertexBuffer;
    RefCntAutoPtr<IBuffer> m_IndexBuffer;
    std::shared_ptr<SF_GLTF::Model> m_Model;
    IRenderDevice* m_pDevice;
    IDeviceContext* m_pContext; 

    std::vector<VertexBuff> m_Vertices;
    std::vector<Uint32> m_Indices;
    std::shared_ptr<SF_GLTF::Mesh> m_Mesh;

    bool m_GPUDataInitialized = false;
};

} // namespace SF_GLTF

} // namespace Diligent