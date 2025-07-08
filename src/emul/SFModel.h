#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "BasicMath.hpp"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"

#include "../pbr/SF_GLTFLoader.hpp"
#include "../pbr/SF_GLTF_PBR_Renderer.hpp"
#include "DynamicMesh.hpp"
#include "terrain.h"

using namespace Diligent;

struct SFModel
{
    std::shared_ptr<SF_GLTF::Model> model;
    SF_GLTF_PBR_Renderer::ModelResourceBindings bindings;
    BoundBox aabb;
    BoundBox worldspaceAABB;
    std::array<SF_GLTF::ModelTransforms, 2> transforms; // [0] - current frame, [1] - previous frame
    float4x4                             modelTransform;
    float                                scale = 1.f;
    float4x4                             scaleAndTransform;
    RefCntAutoPtr<ITextureView> env;
    const SF_GLTF::Node* camera;
    std::vector<const SF_GLTF::Node*> lights;

    std::unique_ptr<SF_GLTF::DynamicMesh> dynamicMesh;
    std::array<SF_GLTF::ModelTransforms, 2> dynamicMeshTransforms; // [0] - current frame, [1] - previous frame

    std::unordered_map<std::string, Uint32> biomMaterialIndex;

    std::vector<SF_GLTF::PlanetType> planetTypes;
}; 