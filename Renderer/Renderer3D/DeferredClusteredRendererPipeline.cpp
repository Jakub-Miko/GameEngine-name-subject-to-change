#include "DeferredClusteredRendererPipeline.h"
#include "RenderPassBuilder.h"
#include "ClusteredRenderer/ActiveClusterFilterPass.h"
#include "ClusteredRenderer/BindlessShadowMappingPass.h"
#include "ClusteredRenderer/ClusteredLightCullingPass.h"
#include "ClusteredRenderer/ClusteredLightingPass.h"
#include "ClusteredRenderer/DebugOverlayPass.h"
#include "CommonRenderPasses/DepthPrepass.h"
#include "CommonRenderPasses/PostProcessingPass.h"
#include "CommonRenderPasses/RenderSubmissionPass.h"
#include "CommonRenderPasses/SkyboxPass.h"
#include "DeferredRenderer/DeferredGeometryPass.h"
#include "DeferredRenderer/GenerateGBufferPass.h"

std::shared_ptr<RenderPipeline> DeferredClusteredRendererPipeline::CreatePipeline() {
    RenderPassBuilder builder;
#ifdef EDITOR
    builder.AddPass(new DebugOverlayPass("RenderOutput", "GBufferMaterial",
        "ClusteredLightLists", "ColorBufferAfterSkybox", "DebugOverlay"));
    builder.AddPass(new PostProcessingPass("ColorBufferAfterSkybox", "DebugOverlay"));
#else
    builder.AddPass(new PostProcessingPass("ColorBuffer"));
#endif

    builder.AddPass(new GenerateGBufferPass("InitialGBuffer", "GBufferMaterial"));
    builder.AddPass(new DepthPrepass("RenderObjects", "SkeletalRenderObjects", "InitialGBuffer", "GBufferAfterPrepass", "DepthBuffer"));
    builder.AddPass(new SkyboxPass("ColorBuffer", "ClusteredLightLists", "ColorBufferAfterSkybox"));
    builder.AddPass(new ClusteredLightCullingPass("RenderLights", "RenderShadowedPointLights",
        "RenderShadowedDirectionalLights", "ShadowCascades",  "ClusteredLightLists", "ActiveClusters"));
    builder.AddPass(new ActiveClusterFilterPass("DepthBuffer", "ActiveClusters"));
    builder.AddPass(new RenderSubmissionPass("RenderObjects", "SkeletalRenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
    builder.AddPass(new DeferredGeometryPass("RenderObjects", "SkeletalRenderObjects", "GBufferAfterPrepass", "RenderOutput"));
    builder.AddPass(new ClusteredLightingPass("RenderOutput", "GBufferMaterial" , "ClusteredLightLists", "RenderShadowedDirectionalLights", "RenderShadowedPointLights", "ColorBuffer", "ShadowsGeneratedTag"
        ,"cubemap_shadow_map_store", "cascaded_shadow_map_store"));
    builder.AddPass(new BindlessShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag",
        "ShadowCascades", "cascaded_shadow_map_store", "cubemap_shadow_map_store"));
    return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
