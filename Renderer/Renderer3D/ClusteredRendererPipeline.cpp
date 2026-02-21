#include "ClusteredRendererPipeline.h"
#include "RenderPassBuilder.h"
#include "ClusteredRenderer/ActiveClusterFilterPass.h"
#include "ClusteredRenderer/BindlessShadowMappingPass.h"
#include "ClusteredRenderer/ClusteredLightCullingPass.h"
#include "ClusteredRenderer/ClusteredLightingPass.h"
#include "CommonRenderPasses/PostProcessingPass.h"
#include "CommonRenderPasses/RenderSubmissionPass.h"
#include "DeferredRenderer/DeferredGeometryPass.h"
#include "DeferredRenderer/DeferredSkeletalGeometryPass.h"
#include "DeferredRenderer/GenerateGBufferPass.h"

std::shared_ptr<RenderPipeline> ClusteredRendererPipeline::CreatePipeline() {
    RenderPassBuilder builder;
    builder.AddPass(new PostProcessingPass("ColorBuffer"));
    builder.AddPass(new GenerateGBufferPass("InitialGBuffer", "GBufferMaterial"));
    builder.AddPass(new ClusteredLightCullingPass("RenderLights", "RenderShadowedPointLights" , "ClusteredLightLists", "ActiveClusters"));
    builder.AddPass(new ActiveClusterFilterPass("GBufferMaterial","RenderOutput" ,"ActiveClusters"));
    builder.AddPass(new RenderSubmissionPass("RenderObjects", "SkeletalRenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
    builder.AddPass(new DeferredGeometryPass("RenderObjects","InitialGBuffer", "RenderMeshOutput"));
    builder.AddPass(new DeferredSkeletalGeometryPass("SkeletalRenderObjects","RenderMeshOutput", "RenderOutput"));
    builder.AddPass(new ClusteredLightingPass("RenderOutput", "GBufferMaterial" , "ClusteredLightLists", "RenderShadowedDirectionalLights", "RenderShadowedPointLights", "ColorBuffer", "ShadowsGeneratedTag", "ShadowCascades"
        ,"cubemap_shadow_map_store", "cascaded_shadow_map_store"));
    builder.AddPass(new BindlessShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag",
        "ShadowCascades", "cascaded_shadow_map_store", "cubemap_shadow_map_store"));
    return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
